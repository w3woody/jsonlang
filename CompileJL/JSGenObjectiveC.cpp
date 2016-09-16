//
//  JSGenObjectiveC.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 8/17/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include "JSGenObjectiveC.h"
#include "JSUtils.h"
#include <stdio.h>
#include <time.h>
#include <set>

static const char * const keywords[] = {
	"auto", "break", "case", "char", "const",
	"continue", "default", "do", "double",
	"else", "enum", "extern", "float", "for",
	"goto", "if", "inline", "int", "long",
	"register", "restrict", "return", "short",
	"signed", "sizeof", "static", "struct",
	"switch", "typedef", "union", "unsigned",
	"void", "volatile", "while", "_Bool",
	"_Complex", "_Imaginery", "BOOL", "Class",
	"bycopy", "byref", "id", "IMP", "in",
	"inout", "nil", "NO", "NULL", "oneway",
	"out", "Protocol", "SEL", "self", "super",
	"YES",

	// reserved for our generator
	"json",

	// reserved fields for NSObject
	"description", "hash", "debugDescription",
	"isProxy", "retain", "release", "autorelease",
	"retainCount", "zone", "class", "superclass",
	"dealloc", "autoContentAccessingProxy",
	"classForArchiver", "classForCoder",
	"classForKeyedArchiver", "classForPortCoder",
	"version", "attributeKeys", "classDescription",
	"toManyRelationshipKeys", "toOneRelationshipKeys",
	"classCode", "className", "scriptingProperties",
	"finalize", NULL
};

/************************************************************************/
/*																		*/
/*	Generate Objective C												*/
/*																		*/
/************************************************************************/

/*	JSGenObjectiveC::JSGenObjectiveC
 *
 *		Construct objective C
 */

JSGenObjectiveC::JSGenObjectiveC(std::string p, std::string pfx, bool ro, ref<JSParser> data)
{
	path = p;
	prefix = pfx;
	parser = data;
	readOnly = ro;
}

JSGenObjectiveC::~JSGenObjectiveC()
{
}

/************************************************************************/
/*																		*/
/*	Construct the files													*/
/*																		*/
/************************************************************************/

/*	JSGenObjectiveC::run
 *
 *		Run
 */

void JSGenObjectiveC::run()
{
	/*
	 *	Generate time for output
	 */

	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	char buffer[128];
	strftime(buffer, sizeof(buffer), "%a %b %d, %Y at %I:%M:%S %p %Z", timeinfo);
	curtime = buffer;

	/*
	 *	Generate files
	 */

	std::map<std::string, ref<JSParserObject> >::iterator it;

	for (it = parser->data.begin(); it != parser->data.end(); ++it) {
		mapNames.clear();

		genHeader(it->first,it->second);
		genSource(it->first,it->second);
	}

	/*
	 *	Now generate a header file which loads all of the headers we just
	 *	wrote out
	 */

	std::string header = genPath("") + ".h";
	FILE *out = fopen(header.c_str(),"w");
	if (out == NULL) {
		fprintf(stderr,"Unable to write file %s\n",header.c_str());
		return;
	}

	fprintf(out,"/*  %s.h\n",prefix.c_str());
	fprintf(out," *\n");
	fprintf(out," *\t\tHeaders for all other automatically generated files\n");
	fprintf(out," *\n");
	fprintf(out," *\t\tAutomatically generated %s\n",curtime.c_str());
	fprintf(out," */\n");
	fprintf(out,"\n");

	for (it = parser->data.begin(); it != parser->data.end(); ++it) {
		fprintf(out,"#import \"%s%s.h\"\n",prefix.c_str(),it->first.c_str());
	}
	fprintf(out,"\n\n");
	fclose(out);
}

/*	JSGenObjectiveC::genObjC
 *
 *		Escape this string for Objective C
 */

std::string JSGenObjectiveC::genObjC(std::string str)
{
	return str;		/* TODO */
}

/*	JSGenObjectiveC::genUnique
 *
 *		Make sure this name is unique
 */

std::string JSGenObjectiveC::genUnique(std::string str)
{
	if (!mapNames.count(str)) return str;

	int index = 1;
	for (;;) {
		char buffer[32];
		sprintf(buffer,"_%d",index++);
		std::string test = str + buffer;
		if (!mapNames.count(test)) return test;
	}
}

std::string JSGenObjectiveC::genTempVar(const char *str, int index)
{
	char buffer[32];
	sprintf(buffer, "%s%d",str,index);
	return genUnique(buffer);
}

static bool hasPrefix(const char *str, const char *prefix)
{
	while ((*str == *prefix) && *str) {
		++str;
		++prefix;
	}
	return *str == 0;
}

static bool isReserved(std::string str)
{
	int index = 0;
	for (index = 0; keywords[index]; ++index) {
		if (str == keywords[index]) return true;
	}
	return false;
}

std::string JSGenObjectiveC::genName(std::string str)
{
	if (mapNames.count(str)) {
		return mapNames[str];
	}

	std::string changeStr;
	if (!IsValidField(changeStr)) {
		/* This is not a valid field name. Convert as best we can */
		size_t i,len = str.size();
		if (len == 0) {
			changeStr.push_back('_');
		} else {
			char c = str[0];
			if (!isalpha(c) && (c != '_')) changeStr.push_back('_');
			for (i = 0; i < len; ++i) {
				c = str[i];
				if (!isalnum(c) && (c != '_')) {
					changeStr.push_back('_');	/* Replace with underscore. */
				} else {
					changeStr.push_back(c);
				}
			}
		}
	} else {
		changeStr = str;
	}
	const char *cstr = str.c_str();

	if (hasPrefix(cstr, "alloc") || hasPrefix(cstr,"init") || hasPrefix(cstr,"copy") || hasPrefix(cstr, "mutableCopy")) {
		changeStr = "field" + std::string(1,toupper(str[0])) + str.substr(1,str.length()-1);
	}
	if (isReserved(changeStr)) {
		changeStr = "field" + std::string(1,toupper(str[0])) + str.substr(1,str.length()-1);
	}

	changeStr = genUnique(changeStr);	/* Make sure generated name is unique */

	mapNames[str] = changeStr;
	return changeStr;
}

std::string JSGenObjectiveC::genPath(std::string objName)
{
	std::string ret = path;
	char end = ret[ret.length()-1];
	if (end != '/') ret += "/";
	ret += prefix;
	ret += objName;
	return ret;
}

/*	JSGenObjectiveC::genHeader
 *
 *		Generate the files
 */

void JSGenObjectiveC::genHeader(std::string objName, ref<JSParserObject> obj)
{
	bool flag = false;

	/*
	 *	Generate the header file name for this file
	 */

	std::string header = genPath(objName) + ".h";
	FILE *out = fopen(header.c_str(),"w");
	if (out == NULL) {
		fprintf(stderr,"Unable to generate file %s\n",header.c_str());
		return;
	}

	/*
	 *	File headers
	 */

	fprintf(out,"/*\t%s%s.h\n",prefix.c_str(),objName.c_str());
	fprintf(out," *\n");
	fprintf(out," *\t\tAutomatically generated %s\n",curtime.c_str());
	fprintf(out," */\n\n");
	fprintf(out,"#import <Foundation/Foundation.h>\n\n");

	/*
	 *	Class forwards
	 */

	flag = false;
	std::map<std::string,ref<JSDeclType> >::iterator iter;
	for (iter = obj->begin(); iter != obj->end(); ++iter) {
		ref<JSDeclType> type = iter->second;
		if (type->type == TYPE_OBJECT) {
			fprintf(out,"@class %s%s;\n",prefix.c_str(),type->symbol.c_str());
			flag = true;
		}
	}
	if (flag) {
		fprintf(out,"\n");
		flag = false;
	}

	/*
	 *	Interface
	 */

	fprintf(out,"@interface %s%s: NSObject\n",prefix.c_str(),objName.c_str());
	fprintf(out,"\n");

	/*
	 *	Generate the property list
	 */

	flag = false;
	for (iter = obj->begin(); iter != obj->end(); ++iter) {
		flag = true;

		ref<JSDeclType> type = iter->second;

		/*
		 *	Figure out if this is an object or a type
		 */

		bool objType = false;
		if ((type->type == TYPE_GENOBJECT) || (type->type == TYPE_OBJECT) || (type->type == TYPE_STRING) || (type->type == TYPE_ARRAY) || (type->isNullable)) {
			objType = true;
		}

		fprintf(out,"@property (nonatomic, %s) ",objType ? "strong" : "assign");

		if (type->type == TYPE_GENOBJECT) {
			fprintf(out,"NSObject *");
		} else if (type->type == TYPE_OBJECT) {
			fprintf(out,"%s%s *",prefix.c_str(),type->symbol.c_str());
		} else if (type->type == TYPE_STRING) {
			fprintf(out,"NSString *");
		} else if (type->type == TYPE_ARRAY) {
			fprintf(out,"NSArray *");

		// Note: all conditionals after isNullable must be the primitives
		} else if (type->isNullable || type->isOptional) {
			fprintf(out,"NSNumber *");
		} else if (type->type == TYPE_REAL) {
			fprintf(out,"double ");
		} else if (type->type == TYPE_INTEGER) {
			fprintf(out,"NSInteger ");
		} else if (type->type == TYPE_BOOLEAN) {
			fprintf(out,"BOOL ");
		}

		bool startComment = false;
		std::string fieldName = genName(iter->first);
		fprintf(out,"%s;",fieldName.c_str());
		if (fieldName != iter->first) {
			startComment = true;
			fprintf(out,"  // %s",iter->first.c_str());
		}

		// Print comment indicating type of array
		if (type->type == TYPE_ARRAY) {
			if (startComment) {
				fprintf(out," -- ");
			} else {
				fprintf(out,"  // ");
			}

			/*
			 *	Now print the type
			 */

			ref<JSDeclType> ptr = type;
			while (ptr->type == TYPE_ARRAY) {
				fprintf(out,"arrayof ");
				ptr = ptr->arrayType;
			}
			if (ptr->type == TYPE_GENOBJECT) {
				fprintf(out,"NSObject *");
			} else if (ptr->type == TYPE_OBJECT) {
				fprintf(out,"%s%s *",prefix.c_str(),ptr->symbol.c_str());
			} else if (ptr->type == TYPE_STRING) {
				fprintf(out,"NSString *");
			// Note: all conditionals after isNullable must be the primitives
			} else if (ptr->isNullable || type->isOptional) {
				fprintf(out,"NSNumber *");
			} else if (ptr->type == TYPE_REAL) {
				fprintf(out,"double ");
			} else if (ptr->type == TYPE_INTEGER) {
				fprintf(out,"NSInteger ");
			} else if (ptr->type == TYPE_BOOLEAN) {
				fprintf(out,"BOOL ");
			}
		}

		fprintf(out,"\n");
	}
	if (flag) {
		fprintf(out,"\n");
		flag = false;
	}

	/*
	 *	Generate the constructors
	 */

	fprintf(out,"- (id)initWithDictionary:(NSDictionary *)dict;\n");
	if (!readOnly) {
		fprintf(out,"- (NSDictionary *)dictionaryRepresentation;\n");
	}
	fprintf(out,"\n@end\n\n");

	fclose(out);
}

static const char *GUtils = \
	"- (id)objectOrNullForKey:(NSString *)key fromDictionary:(NSDictionary *)d\n" \
	"{\n"                                                                     \
	"\tid<NSObject> ret = d[key];\n"                                                    \
	"\treturn ([ret isKindOfClass:[NSNull class]]) \? nil : ret;\n"           \
	"}\n\n";

static const char *GNull = \
	"- (id<NSObject>)nullObject:(id<NSObject>)val\n"                          \
	"{\n"                                                                     \
	"\treturn (val == nil) \? [NSNull null] : val;\n"                        \
	"}\n";

/*	JSGenObjectiveC::genSource
 *
 *		Generate source
 */

void JSGenObjectiveC::genSource(std::string objName, ref<JSParserObject> obj)
{
	bool flag = false;

	/*
	 *	Generate the header file name for this file
	 */

	std::string header = genPath(objName) + ".m";
	FILE *out = fopen(header.c_str(),"w");
	if (out == NULL) {
		fprintf(stderr,"Unable to generate file %s\n",header.c_str());
		return;
	}

	/*
	 *	File headers
	 */

	fprintf(out,"/*\t%s%s.m\n",prefix.c_str(),objName.c_str());
	fprintf(out," *\n");
	fprintf(out," *\t\tAutomatically generated %s\n",curtime.c_str());
	fprintf(out," */\n\n");

	fprintf(out,"#import \"%s%s.h\"\n",prefix.c_str(),objName.c_str());

	/*
	 *	Class header includes
	 */

	std::set<std::string> set;
	std::map<std::string,ref<JSDeclType> >::iterator iter;
	for (iter = obj->begin(); iter != obj->end(); ++iter) {
		ref<JSDeclType> type = iter->second;
		if (type->type == TYPE_OBJECT) {
			set.insert(type->symbol);
		} else if (type->type == TYPE_ARRAY) {
			while (type->isArray) type = type->arrayType;
			if (type->type == TYPE_OBJECT) {
				set.insert(type->symbol);
			}
		}
	}

	if (!set.empty()) {
		std::set<std::string>::iterator setIter;
		for (setIter = set.begin(); setIter != set.end(); ++setIter) {
			fprintf(out,"#import \"%s%s.h\"\n",prefix.c_str(),setIter->c_str());
		}
		fprintf(out,"\n");
	}

	/*
	 *	Generate the contents
	 */

	fprintf(out,"@implementation %s%s\n\n",prefix.c_str(),objName.c_str());

	/*
	 *	Generate the empty constructor
	 */

	if (!readOnly) {
		fprintf(out,"- (id)init\n");
		fprintf(out,"{\n");
		fprintf(out,"\tif (nil != (self = [super init])) {\n");
		fprintf(out,"\t}\n");
		fprintf(out,"\treturn self;\n");
		fprintf(out,"}\n\n");
	}

	/*
	 *	Generat the constructors
	 */

	fprintf(out,"- (id)initWithDictionary:(NSDictionary *)dict\n");
	fprintf(out,"{\n");
	fprintf(out,"\tif (nil != (self = [super init])) {\n");

	/* Generate fetch routines */
	for (iter = obj->begin(); iter != obj->end(); ++iter) {
		ref<JSDeclType> type = iter->second;
		std::string jsonName = genObjC(iter->first);
		std::string fieldName = genName(iter->first);

		fprintf(out,"\t\t// field %s\n",iter->first.c_str());

		if (type->type == TYPE_GENOBJECT) {
			// Always assign
			flag = true;
			fprintf(out,"\t\tself.%s = [self objectOrNullForKey:@\"%s\" fromDictionary:dict];\n",fieldName.c_str(),jsonName.c_str());

		} else if (type->type == TYPE_OBJECT) {
			std::string className = prefix + type->symbol;
			// If nullable, more complex declaration
			if (type->isNullable) {
				flag = true;
				fprintf(out,"\t\t{\n");
				fprintf(out,"\t\t\tNSDictionary *f = [self objectOrNullForKey:@\"%s\" fromDictionary:dict];\n",jsonName.c_str());
				fprintf(out,"\t\t\tif (f) {\n");
				fprintf(out,"\t\t\t\tself.%s = [[%s alloc] initWithDictionary:f];\n",fieldName.c_str(),className.c_str());
				fprintf(out,"\t\t\t}\n");
				fprintf(out,"\t\t}\n");
			} else {
				fprintf(out,"\t\tself.%s = [[%s alloc] initWithDictionary:dict[@\"%s\"]];\n",fieldName.c_str(),className.c_str(),jsonName.c_str());
			}

		} else if (type->type == TYPE_STRING) {
			// If nullable, more complex declaration
			if (type->isNullable) {
				flag = true;
				fprintf(out,"\t\tself.%s = (NSString *)[self objectOrNullForKey:@\"%s\" fromDictionary:dict];\n",fieldName.c_str(),jsonName.c_str());
			} else {
				fprintf(out,"\t\tself.%s = (NSString *)dict[@\"%s\"];\n",fieldName.c_str(),jsonName.c_str());
			}

		} else if (type->type == TYPE_REAL) {
			if (type->isNullable) {
				flag = true;
				fprintf(out,"\t\tself.%s = (NSNumber *)[self objectOrNullForKey:@\"%s\" fromDictionary:dict];\n",fieldName.c_str(),jsonName.c_str());
			} else if (type->isOptional) {
				fprintf(out,"\t\tself.%s = (NSNumber *)dict[@\"%s\"];\n",fieldName.c_str(),jsonName.c_str());
			} else {
				fprintf(out,"\t\t{\n");
				fprintf(out,"\t\t\tNSNumber *n = (NSNumber *)dict[@\"%s\"];\n",jsonName.c_str());
				fprintf(out,"\t\t\tself.%s = [n doubleValue];\n",fieldName.c_str());
				fprintf(out,"\t\t}\n");
			}

		} else if (type->type == TYPE_BOOLEAN) {
			if (type->isNullable) {
				flag = true;
				fprintf(out,"\t\tself.%s = (NSNumber *)[self objectOrNullForKey:@\"%s\" fromDictionary:dict];\n",fieldName.c_str(),jsonName.c_str());
			} else if (type->isOptional) {
				fprintf(out,"\t\tself.%s = (NSNumber *)dict[@\"%s\"];\n",fieldName.c_str(),jsonName.c_str());
			} else {
				fprintf(out,"\t\t{\n");
				fprintf(out,"\t\t\tNSNumber *n = (NSNumber *)dict[@\"%s\"];\n",jsonName.c_str());
				fprintf(out,"\t\t\tself.%s = [n boolValue];\n",fieldName.c_str());
				fprintf(out,"\t\t}\n");
			}

		} else if (type->type == TYPE_INTEGER) {
			if (type->isNullable) {
				flag = true;
				fprintf(out,"\t\tself.%s = (NSNumber *)[self objectOrNullForKey:@\"%s\" fromDictionary:dict];\n",fieldName.c_str(),jsonName.c_str());
			} else if (type->isOptional) {
				fprintf(out,"\t\tself.%s = (NSNumber *)dict[@\"%s\"];\n",fieldName.c_str(),jsonName.c_str());
			} else {
				fprintf(out,"\t\t{\n");
				fprintf(out,"\t\t\tNSNumber *n = (NSNumber *)dict[@\"%s\"];\n",jsonName.c_str());
				fprintf(out,"\t\t\tself.%s = [n integerValue];\n",fieldName.c_str());
				fprintf(out,"\t\t}\n");
			}

		} else if (type->type == TYPE_ARRAY) {
			/* Array */

			/*
			 *	First thing: determine if we're a primitive type or not. If
			 *	at the bottom of the stack is a primitive type, we simply
			 *	copy the array
			 */

			ref<JSDeclType> rootType = type;
			while (rootType->isArray) rootType = rootType->arrayType;
			if (rootType->type != TYPE_OBJECT) {
				/*
				 *	Type does not need translation. Just pull array
				 */

				if (type->isNullable) {
					flag = true;
					fprintf(out,"\t\tself.%s = (NSArray *)[self objectOrNullForKey:@\"%s\" fromDictionary:dict];\n",fieldName.c_str(),jsonName.c_str());
				} else {
					fprintf(out,"\t\tself.%s = dict[@\"%s\"];\n",fieldName.c_str(),jsonName.c_str());
				}
			} else {
				/*
				 *	Type needs translation.
				 */

				flag = true;
				fprintf(out,"\t\t{\n");
				fprintf(out,"\t\t\tNSMutableArray *a0 = [[NSMutableArray alloc] init];\n");
				fprintf(out,"\t\t\tNSArray *s0 = (NSArray *)[self objectOrNullForKey:@\"%s\" fromDictionary:dict];\n",jsonName.c_str());

				fprintf(out,"\t\t\tif (s0) {\n");
				genArrayLoader(out, type->arrayType, 0, 4);
				fprintf(out,"\t\t\t}\n");

				fprintf(out,"\t\t\tself.%s = a0;\n",fieldName.c_str());
				fprintf(out,"\t\t}\n");
			}
		}

		fprintf(out,"\n");
	}

	fprintf(out,"\t}\n");
	fprintf(out,"\treturn self;\n");
	fprintf(out,"}\n");
	fprintf(out,"\n");

	/*
	 *	Generate the utility
	 */

	if (flag) {
		fputs(GUtils, out);
	}

	if (!readOnly) {
		flag = false;
		bool comma = false;
		bool needMutable = false;

		/*
		 *	Generate -description for debugging
		 */

		fprintf(out,"- (NSString *)description\n");
		fprintf(out,"{\n");
		fprintf(out,"    return [[self dictionaryRepresentation] description];\n");
		fprintf(out,"}\n\n");

		/*
		 *	Generate the dictionary construction
		 */

		// TODO: Finish
		fprintf(out,"- (NSDictionary *)dictionaryRepresentation\n");
		fprintf(out,"{\n");

		// First pass: generate local variables with constructed array types
		for (iter = obj->begin(); iter != obj->end(); ++iter) {
			ref<JSDeclType> type = iter->second;
			if (type->isArray) {
				std::string fieldName = genName(iter->first);

				/* Determine if underlying object is an object */
				ref<JSDeclType> rootType = type;
				while (rootType->isArray) rootType = rootType->arrayType;
				if (rootType->type == TYPE_OBJECT) {
					fprintf(out,"\tNSMutableArray *%s_array;\n",fieldName.c_str());

					/* Underlying object is an object to serialize */
					flag |= genArraySave(out, type, 1, fieldName + "_array", "self." + fieldName, 0);

					fprintf(out,"\n");
				}
			}
		}

		// Generate the dictionary to return
		for (iter = obj->begin(); iter != obj->end(); ++iter) {
			ref<JSDeclType> type = iter->second;
			if (type->isOptional) {
				needMutable = true;
				break;
			}
		}

		if (needMutable) {
			fprintf(out,"\tNSMutableDictionary *json = [[NSMutableDictionary alloc] init];\n");

			for (iter = obj->begin(); iter != obj->end(); ++iter) {
				ref<JSDeclType> type = iter->second;
				std::string jsonName = genObjC(iter->first);
				std::string fieldName = genName(iter->first);

				if (type->isOptional) {
					if (type->type == TYPE_ARRAY) {
						ref<JSDeclType> rootType = type;
						while (rootType->isArray) rootType = rootType->arrayType;
						if (rootType->type == TYPE_OBJECT) {
							fprintf(out,"\tif (%s_array) {\n\t",fieldName.c_str());
						} else {
							fprintf(out,"\tif (self.%s) {\n\t",fieldName.c_str());
						}
					} else {
					fprintf(out,"\tif (self.%s) {\n\t",fieldName.c_str());
					}
				}

				fprintf(out,"\tjson[@\"%s\"] = ",jsonName.c_str());
				if (type->isNullable) {
					if (type->type == TYPE_OBJECT) {
						flag = true;
						fprintf(out,"[self nullObject:[self.%s dictionaryRepresentation]]",fieldName.c_str());
					} else if (type->type == TYPE_ARRAY) {
						/*
						 *	Determine what to do; if object, we already declared
						 *	that above.
						 */

						ref<JSDeclType> rootType = type;
						while (rootType->isArray) rootType = rootType->arrayType;
						if (rootType->type == TYPE_OBJECT) {
							fprintf(out,"%s_array",fieldName.c_str());
						} else {
							flag = true;
							fprintf(out,"[self nullObject:self.%s]",fieldName.c_str());
						}

					} else {
						flag = true;
						fprintf(out,"[self nullObject:self.%s]",fieldName.c_str());
					}
				} else {
					if (type->type == TYPE_OBJECT) {
						fprintf(out,"[self.%s dictionaryRepresentation]",fieldName.c_str());
					} else if (type->type == TYPE_ARRAY) {
						/*
						 *	Determine what to do; if object, we already declared
						 *	that above.
						 */

						ref<JSDeclType> rootType = type;
						while (rootType->isArray) rootType = rootType->arrayType;
						if (rootType->type == TYPE_OBJECT) {
							fprintf(out,"%s_array",fieldName.c_str());
						} else {
							fprintf(out,"self.%s",fieldName.c_str());
						}

					} else if (type->type == TYPE_STRING) {
						fprintf(out,"self.%s",fieldName.c_str());
					} else if (type->type == TYPE_GENOBJECT) {
						fprintf(out,"self.%s",fieldName.c_str());
					} else if (type->isOptional) {
						fprintf(out,"self.%s",fieldName.c_str());
						// prim objects below
					} else if (type->type == TYPE_INTEGER) {
						fprintf(out,"@( self.%s )",fieldName.c_str());
					} else if (type->type == TYPE_REAL) {
						fprintf(out,"@( self.%s )",fieldName.c_str());
					} else if (type->type == TYPE_BOOLEAN) {
						fprintf(out,"@( self.%s )",fieldName.c_str());
					}
				}

				fprintf(out,";\n");

				if (type->isOptional) {
					fprintf(out,"\t}\n");
				}
			}

			fprintf(out,"\treturn json;\n");
		} else {
			/*
			 *	All fields are required. We can generate a constant dictionary
			 */

			fprintf(out,"\treturn @{");
			for (iter = obj->begin(); iter != obj->end(); ++iter) {
				if (comma) {
					fprintf(out,",\n");
				} else {
					comma = true;
					fprintf(out,"\n");
				}

				ref<JSDeclType> type = iter->second;
				std::string jsonName = genObjC(iter->first);
				std::string fieldName = genName(iter->first);

				fprintf(out,"\t\t@\"%s\": ",jsonName.c_str());

				if (type->isNullable) {
					if (type->type == TYPE_OBJECT) {
						flag = true;
						fprintf(out,"[self nullObject:[self.%s dictionaryRepresentation]]",fieldName.c_str());
					} else if (type->type == TYPE_ARRAY) {
						/*
						 *	Determine what to do; if object, we already declared
						 *	that above.
						 */

						ref<JSDeclType> rootType = type;
						while (rootType->isArray) rootType = rootType->arrayType;
						if (rootType->type == TYPE_OBJECT) {
							fprintf(out,"%s_array",fieldName.c_str());
						} else {
							flag = true;
							fprintf(out,"[self nullObject:self.%s]",fieldName.c_str());
						}

					} else {
						flag = true;
						fprintf(out,"[self nullObject:self.%s]",fieldName.c_str());
					}
				} else {
					if (type->type == TYPE_OBJECT) {
						fprintf(out,"[self.%s dictionaryRepresentation]",fieldName.c_str());
					} else if (type->type == TYPE_ARRAY) {
						/*
						 *	Determine what to do; if object, we already declared
						 *	that above.
						 */

						ref<JSDeclType> rootType = type;
						while (rootType->isArray) rootType = rootType->arrayType;
						if (rootType->type == TYPE_OBJECT) {
							fprintf(out,"%s_array",fieldName.c_str());
						} else {
							fprintf(out,"self.%s",fieldName.c_str());
						}

					} else if (type->type == TYPE_INTEGER) {
						fprintf(out,"@( self.%s )",fieldName.c_str());
					} else if (type->type == TYPE_REAL) {
						fprintf(out,"@( self.%s )",fieldName.c_str());
					} else if (type->type == TYPE_BOOLEAN) {
						fprintf(out,"@( self.%s )",fieldName.c_str());
					} else if (type->type == TYPE_STRING) {
						fprintf(out,"self.%s",fieldName.c_str());
					} else if (type->type == TYPE_GENOBJECT) {
						fprintf(out,"self.%s",fieldName.c_str());
					}
				}
			}

			fprintf(out,"\n\t};\n");
		}
		fprintf(out,"}\n\n");

		if (flag) {
			fputs(GNull, out);
		}
	}

	fprintf(out,"\n@end\n\n");

	fclose(out);
}

void JSGenObjectiveC::indent(FILE *out, int indent)
{
	while (indent-- > 0) fputc('\t',out);
}

/*
 *	Writes code to assign according to the contents of the array. This generates
 *	a for loop which iterates and converts the contents of the array into the
 *	variable name provided.
 */

bool JSGenObjectiveC::genArraySave(FILE *out, ref<JSDeclType> type, int i, std::string destVar, std::string srcVar, int aix)
{
	bool needClose = false;
	bool needNullDecl = false;
	bool canBeNull = false;

	/*
	 *	At this point the field in destVar has been declared but not set
	 */

	if (type->isNullable || type->isOptional) {
		canBeNull = true;

		indent(out,i);
		fprintf(out,"if (%s == nil) {\n",srcVar.c_str());
		indent(out,i+1);
		fprintf(out,"%s = nil;\n",destVar.c_str());
		indent(out, i);
		fprintf(out,"} else {\n");
		++i;
		needClose = true;
	}

	type = type->arrayType;

	/*
	 *	Initialize the array
	 */

	std::string itype;
	if (type->type == TYPE_OBJECT) {
		itype = prefix + type->symbol;
	} else {
		itype = "NSArray";
	}

	char ibuf[32];
	sprintf(ibuf,"i%d",aix);

	indent(out,i);
	fprintf(out,"%s = [[NSMutableArray alloc] init];\n",destVar.c_str());
	indent(out,i);
	fprintf(out,"for (%s *%s in %s) {\n",itype.c_str(),ibuf,srcVar.c_str());

	if (type->type == TYPE_OBJECT) {
		indent(out, i+1);
		fprintf(out,"[%s addObject:[%s dictionaryRepresentation]];\n",destVar.c_str(),ibuf);
	} else {
		/* Nested array */
		std::string buffer = genTempVar("a", aix+1);
		indent(out, i+1);
		fprintf(out,"NSMutableArray *%s;\n",buffer.c_str());

		needNullDecl |= genArraySave(out, type, i+1, buffer, ibuf, aix+1);

		indent(out, i+1);
		if (canBeNull) {
			needNullDecl = true;
			fprintf(out,"[%s addObject: [self nullObject:%s]];\n",destVar.c_str(),buffer.c_str());
		} else {
			fprintf(out,"[%s addObject: %s];\n",destVar.c_str(),buffer.c_str());
		}
	}

	indent(out,i);
	fprintf(out,"}\n");

	if (needClose) {
		indent(out, i-1);
		fprintf(out,"}\n");
	}

	return needNullDecl;
}

/*
 *	Writes the code for loading the array type. At this point, we have an
 *	array. Note that at the bottom of the stack is an object generation
 *	code
 */

void JSGenObjectiveC::genArrayLoader(FILE *out, ref<JSDeclType> type, int aix, int i)
{
	std::string aname = genTempVar("a", aix);
	std::string sname = genTempVar("s", aix);
	std::string iname = genTempVar("i", aix);

	indent(out,i);
	fprintf(out,"for (id<NSObject> %s in %s) {\n",iname.c_str(),sname.c_str());
	if (type->type == TYPE_ARRAY) {
		/* Generate code to copy an array */
		std::string anew = genTempVar("a", aix+1);
		std::string snew = genTempVar("s", aix+1);

		indent(out, i+1);
		fprintf(out,"NSMutableArray *%s = [[NSMutableArray alloc] init];\n",anew.c_str());
		if (type->isNullable || type->isOptional) {
			indent(out, i+1);
			fprintf(out,"if ((%s != nil) && ![%s isKindOfClass:[NSNull class]]) {\n",iname.c_str(),iname.c_str());
			indent(out, i+2);
			fprintf(out,"NSArray *%s = (NSArray *)%s;\n",snew.c_str(),iname.c_str());
			genArrayLoader(out, type->arrayType, aix+1, i+2);
			indent(out, i+1);
			fprintf(out,"}\n");
		} else {
			indent(out, i+1);
			fprintf(out,"NSArray *%s = (NSArray *)%s;\n",snew.c_str(),iname.c_str());
			genArrayLoader(out, type->arrayType, aix+1, i+1);
		}

		indent(out, i+1);
		fprintf(out,"[%s addObject:%s];\n",aname.c_str(),anew.c_str());
	} else {
		/* Generate code to construct object */
		indent(out,i+1);
		fprintf(out,"[%s addObject:[[%s%s alloc] initWithDictionary:(NSDictionary *)%s]];\n",aname.c_str(),prefix.c_str(),type->symbol.c_str(),iname.c_str());
	}
	indent(out,i);
	fprintf(out,"}\n");
}


