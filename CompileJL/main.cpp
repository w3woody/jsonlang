//
//  main.cpp
//  CompileJL
//
//  Created by William Woody on 8/17/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include <iostream>
#include "JSParser.h"
#include "JSGenObjectiveC.h"
#include "JSGenJava.h"
#include "JSGenGWT.h"

#include <sys/stat.h>
#include <stdio.h>

#define LANGUAGE_OBJECTIVEC			1
#define LANGUAGE_JAVA				2
#define LANGUAGE_GWT				3

static const char *InFile;
static const char *OutFile;
static int Language;
static bool ReadOnly;
static const char *Prefix;

static void runCompiler(FILE *in, const char *pathOut, int language, bool readOnly)
{
	ref<JSInputStream> is = new JSFileInputStream(in);
	ref<JSParser> parser = new JSParser(is);

	if (!parser->run()) return;

	/* Generate appropriate output */
	if (mkdir(pathOut, 0755)) {
		if (errno != EEXIST) {
			fprintf(stderr,"Unable to create directory %s\n",pathOut);
			exit(-1);
		}
	}

	/* Generate the output */
	if (language == LANGUAGE_OBJECTIVEC) {
		ref<JSGenObjectiveC> gen = new JSGenObjectiveC(pathOut,Prefix,readOnly,parser);
		gen->run();
	} else if (language == LANGUAGE_JAVA) {
		ref<JSGenJava> gen = new JSGenJava(pathOut,Prefix,readOnly,parser);
		gen->run();
	} else if (language == LANGUAGE_GWT) {
		ref<JSGenGWT> gen = new JSGenGWT(pathOut,Prefix,readOnly,parser);
		gen->run();
	}
}

static void parseCommandLine(int argc, const char *argv[])
{
	int pos = 1;

	while (pos < argc) {
		const char *str = argv[pos++];

		if (*str == '-') {
			if (!strcmp("-readonly",str)) {
				ReadOnly = true;
			} else if (!strcmp("-objc", str)) {
				Language = LANGUAGE_OBJECTIVEC;
				if (pos >= argc) {
					fprintf(stderr,"Missing parameter\n");
					exit(1);
				}
				Prefix = argv[pos++];
			} else if (!strcmp("-java", str)) {
				Language = LANGUAGE_JAVA;
				if (pos >= argc) {
					fprintf(stderr,"Missing parameter\n");
					exit(1);
				}
				Prefix = argv[pos++];
			} else if (!strcmp("-gwt", str)) {
				Language = LANGUAGE_GWT;
				if (pos >= argc) {
					fprintf(stderr,"Missing parameter\n");
					exit(1);
				}
				Prefix = argv[pos++];
			} else {
				fprintf(stderr,"Illegal parameter %s\n",str);
				exit(1);
			}
		} else if (InFile == NULL) {
			InFile = str;
		} else if (OutFile == NULL) {
			OutFile = str;
		} else {
			fprintf(stderr,"Improper arguments\n");
			exit(-1);
		}
	}

	if (OutFile == NULL) {
		fprintf(stderr,"Insufficient arguments\n");
		exit(-1);
	}
}

int main(int argc, const char * argv[])
{
	parseCommandLine(argc, argv);

	FILE *fin = fopen(InFile,"r");
	if (fin == NULL) {
		fprintf(stdout,"Unable to read %s\n",InFile);
		exit(1);
	}
	runCompiler(fin, OutFile, Language, ReadOnly);

    return 0;
}

