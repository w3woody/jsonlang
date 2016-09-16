//
//  main.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 8/15/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include <iostream>
#include "JSJSONParser.h"
#include "JSInputStream.h"
#include "JSAnalysis.h"

static const char *InFile;
static const char *OutFile;

static void runAnalysis(FILE *in, FILE *out)
{
	ref<JSInputStream> is = new JSFileInputStream(in);
	ref<JSJSONParser> parser = new JSJSONParser(is);
	ref<JSAnalysis> analysis = new JSAnalysis;

	try {
		for (;;) {
			ref<JSONValue> v = parser->parse();
			if (v == NULL) break;
			analysis->addValue(v);
		}
	}
	catch (JSException &ex) {
		fprintf(stderr,"%s\n",ex.what());
		exit(-1);
	}

	ref<JSOutputStream> sout = new JSFileOutputStream(out);
	analysis->generate(sout);
}

static void parseCommandLine(int argc, const char *argv[])
{
	int pos = 1;

	while (pos < argc) {
		const char *str = argv[pos++];

		if (InFile == NULL) {
			InFile = str;
		} else if (OutFile == NULL) {
			OutFile = str;
		} else {
			fprintf(stderr,"Improper arguments");
			exit(-1);
		}
	}

	if (OutFile == NULL) {
		fprintf(stderr,"Insufficient arguments");
		exit(-1);
	}
}

int main(int argc, const char * argv[])
{
	if (argc == 1) {
		runAnalysis(stdin, stdout);
		return 0;
	}

	parseCommandLine(argc, argv);

	FILE *in = fopen(InFile,"r");
	if (in == NULL) {
		fprintf(stderr,"Unable to read %s",InFile);
		exit(-1);
	}

	FILE *out = fopen(OutFile,"w");
	if (out == NULL) {
		fprintf(stderr,"Unable to write %s",OutFile);
		exit(-1);
	}

	runAnalysis(in, out);

	return 0;
}
