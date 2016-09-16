//
//  JSGenGWT.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 9/20/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//
//	Similar to JSGenGWT, except this uses the GWT JSON RTL calls rather
//	than org.json.
//

#include "JSGenGWT.h"
#include "JSUtils.h"
#include <stdio.h>
#include <set>

static const char * const keywords[] = {
	"abstract", "continue", "for", "new", "switch", "assert", "default", 
	"goto", "package", "synchronized", "boolean", "do", "if", "private", 
	"this", "break", "double", "implements", "protected", "throw", "byte", 
	"else", "import", "public", "throws", "case", "enum", "instanceof", 
	"return", "transient", "catch", "extends", "int", "short", "try", 
	"char", "final", "interface", "static", "void", "class", "finally", 
	"long", "strictfp", "volatile", "const", "float", "native", "super", 
	"while", NULL
};

/************************************************************************/
/*																		*/
/*	Generate Objective C												*/
/*																		*/
/************************************************************************/

/*	JSGenGWT::JSGenGWT
 *
 *		Construct objective C
 */

JSGenGWT::JSGenGWT(std::string p, std::string pfx, bool ro, ref<JSParser> data)
{
	path = p;
	prefix = pfx;
	parser = data;
	readOnly = ro;
}

JSGenGWT::~JSGenGWT()
{
}

/************************************************************************/
/*																		*/
/*	Construct the files													*/
/*																		*/
/************************************************************************/

/*	JSGenGWT::run
 *
 *		Run
 */

void JSGenGWT::run()
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

		genSource(it->first,it->second);
	}
}

/*	JSGenGWT::genObjC
 *
 *		Escape this string for Objective C
 */

std::string JSGenGWT::genJava(std::string str)
{
	return str;		/* TODO */
}

/*	JSGenGWT::genUnique
 *
 *		Make sure this name is unique
 */

std::string JSGenGWT::genUnique(std::string str)
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

std::string JSGenGWT::genTempVar(const char *str, int index)
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

std::string JSGenGWT::genName(std::string str)
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
			bool capLetter = false;
			bool firstChar = true;
			char c = str[0];
			if (!isalpha(c) && (c != '_')) changeStr.push_back('_');
			for (i = 0; i < len; ++i) {
				c = str[i];
				if (firstChar) {
					firstChar = false;
					c = tolower(c);
				}
				if (!isalnum(c) && (c != '_')) {
					changeStr.push_back('_');	/* Replace with underscore. */
				} else if (c == '_') {
					capLetter = true;
				} else {
					if (capLetter) {
						capLetter = false;
						c = toupper(c);
					}
					changeStr.push_back(c);
				}
			}
		}
	} else {
		changeStr = str;
	}
	const char *cstr = str.c_str();

	if (hasPrefix(cstr, "get") || hasPrefix(cstr,"set") || hasPrefix(cstr,"is")) {
		changeStr = "field" + std::string(1,toupper(str[0])) + str.substr(1,str.length()-1);
	}
	if (isReserved(changeStr)) {
		changeStr = "field" + std::string(1,toupper(str[0])) + str.substr(1,str.length()-1);
	}

	changeStr = genUnique(changeStr);	/* Make sure generated name is unique */

	mapNames[str] = changeStr;
	return changeStr;
}

std::string JSGenGWT::genCapName(std::string str)
{
	std::string ret = genName(str);
	return std::string(1,toupper(ret[0])) + ret.substr(1,str.length()-1);
}

std::string JSGenGWT::genPath(std::string objName)
{
	std::string ret = path;
	char end = ret[ret.length()-1];
	if (end != '/') ret += "/";
	ret += objName;
	return ret;
}

void JSGenGWT::indent(FILE *out, int indent)
{
	for (int i = 0; i < indent; ++i) fprintf(out,"    ");
}

/*
 *	Generate java type from variable
 */

std::string JSGenGWT::varType(ref<JSDeclType> type, bool subType)
{
	if (type->type == TYPE_ARRAY) {
		// Recursively construct list array description
		std::string ret = "List<";
		ret = ret + varType(type->arrayType,true);
		ret = ret + ">";
		return ret;

	} else if (type->type == TYPE_GENOBJECT) {
		return "JSONValue";
	} else if (type->type == TYPE_OBJECT) {
		return type->symbol;
	} else if (type->type == TYPE_STRING) {
		return "String";
	} else if (type->isNullable || type->isOptional || subType) {
		if (type->type == TYPE_REAL) {
			return "Double";
		} else if (type->type == TYPE_INTEGER) {
			return "Integer";
		} else if (type->type == TYPE_BOOLEAN) {
			return "Boolean";
		} else {
			return "???";
		}
	} else {
		if (type->type == TYPE_REAL) {
			return "double";
		} else if (type->type == TYPE_INTEGER) {
			return "int";
		} else if (type->type == TYPE_BOOLEAN) {
			return "boolean";
		} else {
			return "???";
		}
	}
}

/* Return spaces for indent, with 0 being function level. Note we need to add
 * 4 spaces to what is returned here; this is deliberate for code formatting */
static const char *indentStr(int level)
{
	// 40 spaces                 0123456789012345678901234567890123456789
	static const char *spaces = "                                        ";
	level = 4 + level * 4;
	return spaces + (40 - level);
}

/*	JSGenObjectiveC::genSource
 *
 *		Generate source
 */

void JSGenGWT::genSource(std::string objName, ref<JSParserObject> obj)
{
	/*
	 *	Generate the header file name for this file
	 */

	std::string header = genPath(objName) + ".java";
	FILE *out = fopen(header.c_str(),"w");
	if (out == NULL) {
		fprintf(stderr,"Unable to generate file %s\n",header.c_str());
		return;
	}

	/*
	 *	File headers
	 */

	fprintf(out,"/*\t%s.java\n",objName.c_str());
	fprintf(out," *\n");
	fprintf(out," *\t\tAutomatically generated %s\n",curtime.c_str());
	fprintf(out," */\n\n");

	/*
	 *	Generate the package prefix, import declarations
	 */

	fprintf(out,"package %s;\n\n",prefix.c_str());
	fprintf(out,"import com.google.gwt.json.client.*;\n");

	/*
	 *	Determine if I need the list interface. We need that in the event
	 *	we have an array field. Note that generic 'objects' are handled
	 *	as a generic Java 'Object' which does not need to be imported
	 */

	std::map<std::string,ref<JSDeclType> >::iterator iter;
	for (iter = obj->begin(); iter != obj->end(); ++iter) {
		ref<JSDeclType> type = iter->second;
		if (type->type == TYPE_ARRAY) {
			fprintf(out,"import java.util.List;\n");
			fprintf(out,"import java.util.ArrayList;\n");
			break;
		}
	}

	/*
	 *	Run through and generate imports for all of the classes which
	 *	I may touch.
	 */

	std::set<std::string> set;
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
			fprintf(out,"import %s.%s;\n",prefix.c_str(),setIter->c_str());
		}
	}
	fprintf(out,"\n");

	/*
	 *	Generate the class prefix
	 */

	fprintf(out,"public class %s\n",objName.c_str());
	fprintf(out,"{\n");

	/*
	 *	Generate the fields
	 */

	for (iter = obj->begin(); iter != obj->end(); ++iter) {
		ref<JSDeclType> type = iter->second;
		std::string fieldName = genName(iter->first);

		fprintf(out,"    private %s %s;",varType(type).c_str(),fieldName.c_str());
		if (fieldName != iter->first) {
			fprintf(out,"  // %s",iter->first.c_str());
		}
		fprintf(out,"\n");
	}
	fprintf(out,"\n");

	/*
	 *	Generic empty constructor
	 */
	if (!readOnly) {
		fprintf(out,"    public %s()\n",objName.c_str());
		fprintf(out,"    {\n");
		fprintf(out,"    }\n");
		fprintf(out,"\n");
	}

	/*
	 *	Generate the JSON constructor
	 */

	fprintf(out,"    public %s(JSONObject json) throws JSONException\n",objName.c_str());
	fprintf(out,"    {\n");

	/*
	 *	For the purposes of construction we use a temporary variable to
	 *	hold the value as we pull it out. This creates it if we need it
	 */

	std::string tmpVar;
	for (iter = obj->begin(); iter != obj->end(); ++iter) {
		ref<JSDeclType> type = iter->second;
		if (type->type != TYPE_GENOBJECT) {
			tmpVar = genUnique("tmp");
			fprintf(out,"        JSONValue %s;\n\n",tmpVar.c_str());
			break;
		}
	}


	bool flag = false;
	for (iter = obj->begin(); iter != obj->end(); ++iter) {
		if (flag) {
			fprintf(out,"\n");
		} else {
			flag = true;
		}

		ref<JSDeclType> type = iter->second;
		std::string jsonName = genJava(iter->first);
		std::string fieldName = genName(iter->first);

		fprintf(out,"        // %s\n",jsonName.c_str());

		/*
		 *	GWT api uses JSONNull for null and null for optional not present
		 *	We do not distinguish between "not present" and "null", so if this
		 *	is nullable, we siply convert JSONNull into null
		 */

		if (type->type == TYPE_GENOBJECT) {
			fprintf(out,"        %s = json.get(\"%s\");\n",fieldName.c_str(),jsonName.c_str());
			if (type->isNullable) {
				fprintf(out,"        if ((%s != null) && (%s.isNull() != null)) {\n",fieldName.c_str(),fieldName.c_str());
				fprintf(out,"            %s = null;\n",fieldName.c_str());
				fprintf(out,"        }\n");
			}

		} else {
			int indent = 0;

			if (type->isOptional) {
				fprintf(out,"        if (json.containsKey(\"%s\")) {\n",jsonName.c_str());
				++indent;
			}
			fprintf(out,"%s    %s = json.get(\"%s\");\n",indentStr(indent),tmpVar.c_str(),jsonName.c_str());
			if (type->isNullable) {
				fprintf(out,"%s    if (%s.isNull() != null) {\n",indentStr(indent),tmpVar.c_str());
				++indent;
			}

			/* 
			 * By the time we get here, the contents of our temporary variable is set,
			 * so we can simply assign
			 */

			if (type->type == TYPE_OBJECT) {
				fprintf(out,"%s    %s = new %s(%s.isObject());\n",indentStr(indent),fieldName.c_str(),type->symbol.c_str(),tmpVar.c_str());

			} else if (type->type == TYPE_ARRAY) {
				std::string tmpsrc = genTempVar("a", 1);
				fprintf(out,"%s    {\n",indentStr(indent));
				fprintf(out,"%s        JSONArray %s = %s.isArray();\n",indentStr(indent),tmpsrc.c_str(),tmpVar.c_str());
				genArrayLoader(out, fieldName, type, tmpsrc, 1, indent+3);
				fprintf(out,"%s    }\n",indentStr(indent));

			} else if (type->type == TYPE_STRING) {
				fprintf(out,"%s    %s = %s.isString().stringValue();\n",indentStr(indent),fieldName.c_str(),tmpVar.c_str());

			} else if (type->type == TYPE_INTEGER) {
				fprintf(out,"%s    %s = (int)(%s.isNumber().doubleValue());\n",indentStr(indent),fieldName.c_str(),tmpVar.c_str());

			} else if (type->type == TYPE_REAL) {
				fprintf(out,"%s    %s = %s.isNumber().doubleValue();\n",indentStr(indent),fieldName.c_str(),tmpVar.c_str());

			} else if (type->type == TYPE_BOOLEAN) {
				fprintf(out,"%s    %s = %s.isBoolean().booleanValue();\n",indentStr(indent),fieldName.c_str(),tmpVar.c_str());
			}

			while (indent > 0) {
				fprintf(out,"%s    }\n",indentStr(--indent));
			}
		}
	}

	fprintf(out,"    }\n");
	fprintf(out,"\n");

	/*
	 *	Generate code to encode
	 */

	std::string value = genUnique("value");			/* Prevent name collision */
	if (!readOnly) {
		fprintf(out,"    public JSONObject toJSON() throws JSONException\n");
		fprintf(out,"    {\n");
		fprintf(out,"        JSONObject %s = new JSONObject();\n",value.c_str());
		fprintf(out,"\n");

		for (iter = obj->begin(); iter != obj->end(); ++iter) {
			ref<JSDeclType> type = iter->second;
			std::string fieldName = genName(iter->first);
			std::string jsonName = genJava(iter->first);

			fprintf(out,"        // %s\n",jsonName.c_str());

			if (type->isNullable || type->isOptional) {
				if (type->type == TYPE_OBJECT) {
					fprintf(out,"        if (%s != null) {\n",fieldName.c_str());
					fprintf(out,"            %s.put(\"%s\",%s.toJSON());\n",value.c_str(),jsonName.c_str(),fieldName.c_str());
					if (!type->isOptional) {
						fprintf(out,"        } else {\n");
						fprintf(out,"            %s.put(\"%s\",JSONNull.getInstance());\n",value.c_str(),jsonName.c_str());
					}
					fprintf(out,"        }\n");
				} else if (type->type == TYPE_ARRAY) {
					std::string str = genTempVar("a", 0);

					// TODO
					fprintf(out,"        if (%s != null) {\n",fieldName.c_str());
					fprintf(out,"            JSONArray %s = new JSONArray();\n",str.c_str());

					genArraySave(out, str, type->arrayType, fieldName, 0, 3);

					fprintf(out,"            %s.put(\"%s\",%s);\n",value.c_str(),jsonName.c_str(),str.c_str());
					if (!type->isOptional) {
						fprintf(out,"        } else {\n");
						fprintf(out,"            %s.put(\"%s\",JSONNull.getInstance());\n",value.c_str(),jsonName.c_str());
					}
					fprintf(out,"        }\n");
				} else if (type->type == TYPE_GENOBJECT) {
					fprintf(out,"        if (%s != null) {\n",fieldName.c_str());
					fprintf(out,"            %s.put(\"%s\",%s);\n",value.c_str(),jsonName.c_str(),fieldName.c_str());
					if (!type->isOptional) {
						fprintf(out,"        } else {\n");
						fprintf(out,"            %s.put(\"%s\",JSONNull.getInstance());\n",value.c_str(),jsonName.c_str());
					}
					fprintf(out,"        }\n");
				} else if (type->type == TYPE_STRING) {
					fprintf(out,"        if (%s != null) {\n",fieldName.c_str());
					fprintf(out,"            %s.put(\"%s\",new JSONString(%s));\n",value.c_str(),jsonName.c_str(),fieldName.c_str());
					if (!type->isOptional) {
						fprintf(out,"        } else {\n");
						fprintf(out,"            %s.put(\"%s\",JSONNull.getInstance());\n",value.c_str(),jsonName.c_str());
					}
					fprintf(out,"        }\n");
				} else if (type->type == TYPE_REAL) {
					fprintf(out,"        if (%s != null) {\n",fieldName.c_str());
					fprintf(out,"            %s.put(\"%s\",new JSONNumber(%s.doubleValue()));\n",value.c_str(),jsonName.c_str(),fieldName.c_str());
					if (!type->isOptional) {
						fprintf(out,"        } else {\n");
						fprintf(out,"            %s.put(\"%s\",JSONNull.getInstance());\n",value.c_str(),jsonName.c_str());
					}
					fprintf(out,"        }\n");
				} else if (type->type == TYPE_INTEGER) {
					fprintf(out,"        if (%s != null) {\n",fieldName.c_str());
					fprintf(out,"            %s.put(\"%s\",new JSONNumber(%s.intValue()));\n",value.c_str(),jsonName.c_str(),fieldName.c_str());
					if (!type->isOptional) {
						fprintf(out,"        } else {\n");
						fprintf(out,"            %s.put(\"%s\",JSONNull.getInstance());\n",value.c_str(),jsonName.c_str());
					}
					fprintf(out,"        }\n");
				} else if (type->type == TYPE_BOOLEAN) {
					fprintf(out,"        if (%s != null) {\n",fieldName.c_str());
					fprintf(out,"            %s.put(\"%s\",JSONBoolean.getInstance(%s.booleanValue()));\n",value.c_str(),jsonName.c_str(),fieldName.c_str());
					if (!type->isOptional) {
						fprintf(out,"        } else {\n");
						fprintf(out,"            %s.put(\"%s\",JSONNull.getInstance());\n",value.c_str(),jsonName.c_str());
					}
					fprintf(out,"        }\n");
				}
			} else {
				/*
				 *	Value is not nullable, so direct put
				 */

				if (type->type == TYPE_OBJECT) {
					fprintf(out,"        %s.put(\"%s\",%s.toJSON());\n",value.c_str(),jsonName.c_str(),fieldName.c_str());
				} else if (type->type == TYPE_ARRAY) {
					std::string str = genTempVar("a", 0);
					// TODO
					fprintf(out,"        {\n");
					fprintf(out,"            JSONArray %s = new JSONArray();\n",str.c_str());

					genArraySave(out, str, type->arrayType, fieldName, 0, 3);

					fprintf(out,"            %s.put(\"%s\",%s);\n",value.c_str(),jsonName.c_str(),str.c_str());
					fprintf(out,"        }\n");

				} else if (type->type == TYPE_GENOBJECT) {
					fprintf(out,"        %s.put(\"%s\",%s);\n",value.c_str(),jsonName.c_str(),fieldName.c_str());
				} else if (type->type == TYPE_STRING) {
					fprintf(out,"        %s.put(\"%s\",new JSONString(%s));\n",value.c_str(),jsonName.c_str(),fieldName.c_str());
				} else if (type->type == TYPE_REAL) {
					fprintf(out,"        %s.put(\"%s\",new JSONNumber(%s));\n",value.c_str(),jsonName.c_str(),fieldName.c_str());
				} else if (type->type == TYPE_INTEGER) {
					fprintf(out,"        %s.put(\"%s\",new JSONNumber(%s));\n",value.c_str(),jsonName.c_str(),fieldName.c_str());
				} else if (type->type == TYPE_BOOLEAN) {
					fprintf(out,"        %s.put(\"%s\",JSONBoolean.getInstance(%s));\n",value.c_str(),jsonName.c_str(),fieldName.c_str());
				}
			}

			fprintf(out,"\n");
		}

		fprintf(out,"        return %s;\n",value.c_str());
		fprintf(out,"    }\n");
		fprintf(out,"\n");
	}

	/*
	 *	Generate the field accessors
	 */

	for (iter = obj->begin(); iter != obj->end(); ++iter) {
		ref<JSDeclType> type = iter->second;
		std::string fieldName = genName(iter->first);
		std::string capName = genCapName(fieldName);

		fprintf(out,"\n");
		if (fieldName != iter->first) {
			fprintf(out,"    /**\n");
			fprintf(out,"     *  Get field %s\n",iter->first.c_str());
			fprintf(out,"     */\n");
		}

		const char *fprefix = (iter->second->type == TYPE_BOOLEAN) ? "is" : "get";
		fprintf(out,"    public %s %s%s()\n",varType(iter->second).c_str(),fprefix,capName.c_str());
		fprintf(out,"    {\n");
		fprintf(out,"        return %s;\n",fieldName.c_str());
		fprintf(out,"    }\n");

		if (!readOnly) {
			fprintf(out,"\n");
			if (fieldName != iter->first) {
				fprintf(out,"    /**\n");
				fprintf(out,"     *  Set field %s\n",iter->first.c_str());
				fprintf(out,"     */\n");
			}

			fprintf(out,"    public void set%s(%s %s)\n",capName.c_str(),varType(iter->second).c_str(),value.c_str());
			fprintf(out,"    {\n");
			fprintf(out,"        %s = %s;\n",fieldName.c_str(),value.c_str());
			fprintf(out,"    }\n");
		}
	}

	fprintf(out,"}\n");
}

void JSGenGWT::genArraySave(FILE *out, std::string destVar, ref<JSDeclType> srcType, std::string srcVar, int aix, int i)
{
	std::string index = genTempVar("i", aix);
	std::string ct = genTempVar("ct", aix);

	indent(out,i);
	fprintf(out,"int %s = 0;\n",ct.c_str());
	indent(out,i);
	fprintf(out,"for (%s %s: %s) {\n",varType(srcType).c_str(),index.c_str(),srcVar.c_str());

	/*
	 *	If nullable, wrap
	 */

	++i;
	if (srcType->isNullable) {
		indent(out,i);
		fprintf(out,"if (%s == null) {\n",index.c_str());
		indent(out,i+1);
		fprintf(out,"%s.set(%s++,JSONNull.getInstance());\n",destVar.c_str(),ct.c_str());
		indent(out,i);
		fprintf(out,"} else {\n");

		++i;
	}

	if (srcType->type == TYPE_ARRAY) {
		std::string a = genTempVar("a", aix+1);
		indent(out,i);
		fprintf(out,"JSONArray %s = new JSONArray();\n",a.c_str());
		genArraySave(out, a, srcType->arrayType, index, aix+1, i+1);
		indent(out,i);
		fprintf(out,"%s.set(%s++,%s);\n", destVar.c_str(),ct.c_str(),a.c_str());

	} else if (srcType->type == TYPE_OBJECT) {
		indent(out,i);
		fprintf(out,"%s.set(%s++,%s.toJSON());\n",destVar.c_str(),ct.c_str(),index.c_str());

	} else if (srcType->type == TYPE_GENOBJECT) {
		indent(out,i);
		fprintf(out,"%s.set(%s++,%s);\n",destVar.c_str(),ct.c_str(),index.c_str());

	} else if (srcType->type == TYPE_STRING) {
		indent(out,i);
		fprintf(out,"%s.set(%s++,new JSONString(%s));\n",destVar.c_str(),ct.c_str(),index.c_str());

	} else if (srcType->type == TYPE_INTEGER) {
		indent(out,i);
		if (srcType->isNullable) {
			fprintf(out,"%s.set(%s++,new JSONNumber(%s.intValue()));\n",destVar.c_str(),ct.c_str(),index.c_str());
		} else {
			fprintf(out,"%s.set(%s++,new JSONNumber(%s));\n",destVar.c_str(),ct.c_str(),index.c_str());
		}

	} else if (srcType->type == TYPE_REAL) {
		indent(out,i);
		if (srcType->isNullable) {
			fprintf(out,"%s.set(%s++,new JSONNumber(%s.doubleValue()));\n",destVar.c_str(),ct.c_str(),index.c_str());
		} else {
			fprintf(out,"%s.set(%s++,new JSONNumber(%s));\n",destVar.c_str(),ct.c_str(),index.c_str());
		}

	} else if (srcType->type == TYPE_BOOLEAN) {
		indent(out,i);
		if (srcType->isNullable) {
			fprintf(out,"%s.set(%s++,JSONBoolean.getInstance(%s.booleanValue()));\n",destVar.c_str(),ct.c_str(),index.c_str());
		} else {
			fprintf(out,"%s.set(%s++,JSONBoolean.getInstance(%s));\n",destVar.c_str(),ct.c_str(),index.c_str());
		}
	}

	if (srcType->isNullable) {
		--i;
		indent(out,i);
		fprintf(out,"}\n");
	}
	--i;

	indent(out,i);
	fprintf(out,"}\n");
}

/*	JSGenGWT::genArrayLoader
 *
 *		Write the array
 */

void JSGenGWT::genArrayLoader(FILE *out, std::string destVar, ref<JSDeclType> srcType, std::string srcVar, int aix, int i)
{
	std::string index = genTempVar("i", aix);
	std::string length = genTempVar("l", aix);
	std::string tmp = genTempVar("t", aix);

	indent(out,i);
	fprintf(out,"%s = new ArrayList<%s>();\n",destVar.c_str(),varType(srcType->arrayType,true).c_str());

	indent(out,i);
	fprintf(out,"int %s = %s.size();\n",length.c_str(),srcVar.c_str());

	indent(out, i);
	fprintf(out,"for (int %s = 0; %s < %s; ++%s) {\n",index.c_str(),index.c_str(),length.c_str(),index.c_str());

	ref<JSDeclType> type = srcType->arrayType;
	indent(out,i+1);
	fprintf(out,"JSONValue %s = %s.get(%s);\n",tmp.c_str(),srcVar.c_str(),index.c_str());

	if (type->isNullable) {
		indent(out,i+1);
		fprintf(out,"if (%s.isNull() != null) {\n",tmp.c_str());
		indent(out,i+2);
		fprintf(out,"%s.add(null);\n",destVar.c_str());
		indent(out,i+1);
		fprintf(out,"} else {\n");
		++i;
	}

	if (type->type == TYPE_GENOBJECT) {
		indent(out,i+1);
		fprintf(out,"%s.add(%s);\n",destVar.c_str(),tmp.c_str());

	} else if (type->type == TYPE_OBJECT) {
		indent(out,i+1);
		fprintf(out,"%s.add(new %s(%s.isObject()));\n",destVar.c_str(),type->symbol.c_str(),tmp.c_str());

	} else if (type->type == TYPE_STRING) {
		indent(out,i+1);
		fprintf(out,"%s.add(%s.isString().stringValue());\n",destVar.c_str(),tmp.c_str());

	} else if (type->type == TYPE_BOOLEAN) {
		indent(out,i+1);
		fprintf(out,"%s.add(%s.isBoolean().booleanValue());\n",destVar.c_str(),tmp.c_str());

	} else if (type->type == TYPE_REAL) {
		indent(out,i+1);
		fprintf(out,"%s.add(%s.isNumber().doubleValue());\n",destVar.c_str(),tmp.c_str());

	} else if (type->type == TYPE_INTEGER) {
		indent(out,i+1);
		fprintf(out,"%s.add((int)(%s.isNumber().doubleValue()));\n",destVar.c_str(),tmp.c_str());

	} else if (type->type == TYPE_ARRAY) {
		std::string field = genTempVar("a",aix+1);
		indent(out,i+1);
		fprintf(out,"JSONArray %s = %s.isArray();\n",field.c_str(),tmp.c_str());
		std::string destField = genTempVar("o",aix+1);
		indent(out,i+1);
		fprintf(out,"%s %s;\n",varType(type).c_str(),destField.c_str());
		genArrayLoader(out, destField.c_str(), type, field.c_str(), aix+1, i+1);
		indent(out,i+1);
		fprintf(out,"%s.add(%s);\n",destVar.c_str(),destField.c_str());
	}

	if (type->isNullable) {
		--i;
		indent(out,i+1);
		fprintf(out,"}\n");
	}

	indent(out, i);
	fprintf(out,"}\n");
}

