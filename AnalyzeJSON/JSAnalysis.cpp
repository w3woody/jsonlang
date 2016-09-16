//
//  JSAnalysis.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 8/16/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include "JSAnalysis.h"
#include "JSUtils.h"
#import <unordered_set>

/************************************************************************/
/*																		*/
/*	JSDeclTemplate														*/
/*																		*/
/************************************************************************/

JSDeclTemplate::JSDeclTemplate(ref<JSDeclTemplate> ref, const std::string &n)
{
	declType = ref->declType;
	isNullable = ref->isNullable;
	isOptional = ref->isOptional;
	isReal = ref->isReal;
	name = n;
	arrayType = ref->arrayType;
	object = ref->object;
	illegalList = ref->illegalList;
}

JSDeclTemplate::JSDeclTemplate(ref<JSONValue> val)
{
	isOptional = false;

	if (val->getJSONType() == JSONTYPE_NULL) {
		declType = JSONTYPE_UNDEFINED;
		isNullable = true;

	} else {
		declType = val->getJSONType();
		isNullable = false;

		if (declType == JSONTYPE_ARRAY) {
			/*
			 *	Get the array type
			 */

			JSONArray *a = (JSONArray *)val.get();
			size_t len = a->size();
			if (len == 0) {
				arrayType = new JSDeclTemplate;
			} else {
				/*
				 *	Run through and merge all the types into a single
				 *	unified type
				 */

				arrayType = NULL;
				int i;
				for (i = 0; i < len; ++i) {
					ref<JSDeclTemplate> tmp = new JSDeclTemplate((*a)[i]);
					if (arrayType == NULL) {
						arrayType = tmp;
					} else {
						arrayType->merge(tmp);
					}
				}
			}
		} else if (declType == JSONTYPE_OBJECT) {
			/*
			 *	Run and get the type of the object, storing it into the
			 *	object type
			 */

			JSONObject *o = (JSONObject *)val.get();
			std::map<std::string,ref<JSONValue> >::iterator it;
			for (it = o->begin(); it != o->end(); ++it) {
				ref<JSDeclTemplate> type = new JSDeclTemplate(it->second);
				object[it->first] = type;
			}

		} else if (declType == JSONTYPE_NUMBER) {
			JSONNumber *number = (JSONNumber *)val.get();

			isReal = !number->isInteger();
		}
	}
}

void JSDeclTemplate::merge(ref<JSDeclTemplate> type)
{
	isNullable |= type->isNullable;
	isOptional |= type->isOptional;
	isReal |= type->isReal;

	if (declType == JSONTYPE_UNDEFINED) {
		/*
		 *	Undefined; bring in type source
		 */

		declType = type->declType;
		arrayType = type->arrayType;
		object = type->object;
		illegalList = type->illegalList;

	} else if (type->declType == JSONTYPE_UNDEFINED) {
		/*
		 *	Nothing to do here
		 */

	} else if (declType == JSONTYPE_ILLEGAL) {
		/*
		 *	Illegal type
		 */

		if (type->declType == JSONTYPE_ILLEGAL) {
			std::vector<ref<JSDeclTemplate> >::iterator i;
			for (i = type->illegalList.begin(); i != type->illegalList.end(); ++i) {
				illegalList.push_back(*i);
			}
		} else {
			illegalList.push_back(type);
		}

	} else if (declType != type->declType) {
		/*
		 *	Types don't match; declaration type
		 */

		ref<JSDeclTemplate> copy = new JSDeclTemplate;
		copy->declType = declType;
		copy->isNullable = isNullable;
		copy->arrayType = arrayType;
		copy->object = object;
		copy->illegalList = illegalList;

		declType = JSONTYPE_ILLEGAL;
		illegalList.push_back(copy);

		if (type->declType == JSONTYPE_ILLEGAL) {
			std::vector<ref<JSDeclTemplate> >::iterator i;
			for (i = type->illegalList.begin(); i != type->illegalList.end(); ++i) {
				illegalList.push_back(*i);
			}
		} else {
			illegalList.push_back(type);
		}
	} else {
		/*
		 *	Types are equal. Merge
		 */

		if (declType == JSONTYPE_ARRAY) {
			/* Merge array types */
			arrayType->merge(type->arrayType);

		} else if (declType == JSONTYPE_OBJECT) {
			/* Merge objects */
			std::unordered_set<std::string> fields;
			std::map<std::string, ref<JSDeclTemplate> >::iterator it;

			/* Step 1: insert from the first side */
			for (it = object.begin(); it != object.end(); ++it) {
				fields.insert(it->first);
			}

			/* Step 2: Merge the fields, and note those which are not on both sides */
			for (it = type->object.begin(); it != type->object.end(); ++it) {
				/* Fields: remove if already there, add if not. */
				if (fields.count(it->first) == 0) {
					fields.insert(it->first);
				} else {
					fields.erase(it->first);
				}

				/* Now merge the types */
				ref<JSDeclTemplate> &srcRef = object[it->first];
				if (srcRef == NULL) {
					srcRef = it->second;
				} else {
					srcRef->merge(it->second);
				}
			}

			/* Step 3: mark all fields that are not in common as optional */
			std::unordered_set<std::string>::iterator fiter;
			for (fiter = fields.begin(); fiter != fields.end(); ++fiter) {
				ref<JSDeclTemplate> ref = object[*fiter];
				ref->isOptional = true;
			}
		}
	}
}

/*
 *	Return true if the two types are compatible: that is, if they can be
 *	merged into a single type. This is actually used to collapse types. Note
 *	that this is more stringent than just if two sides can be merged; this
 *	will also fail if two object declarations don't have the same fields.
 */

bool JSDeclTemplate::isCompatible(ref<JSDeclTemplate> type)
{
	/* One or the other is undefined, so this is assumed as compatible */
	if ((type->declType == JSONTYPE_UNDEFINED) || (declType == JSONTYPE_UNDEFINED)) {
		return true;
	}

	/* If either side is illegal, this fails */
	if ((type->declType == JSONTYPE_ILLEGAL) || (declType == JSONTYPE_ILLEGAL)) {
		return false;
	}

	/* The types are not compatible */
	if (type->declType != declType) return false;

	/* Compatible types. If array, also determine if the contents are 
	 * compatible
	 */

	if (declType == JSONTYPE_ARRAY) {
		return arrayType->isCompatible(type->arrayType);
	}

	if (declType == JSONTYPE_OBJECT) {
		/*
		 *	First, build a list of all the objects, and fail if the
		 *	object keys are not the same
		 */

		std::map<std::string, ref<JSDeclTemplate> >::iterator it;
		std::unordered_set<std::string> fields;

		for (it = type->object.begin(); it != type->object.end(); ++it) {
			fields.insert(it->first);
		}
		for (it = object.begin(); it != object.end(); ++it) {
			if (fields.count(it->first) == 0) return false;
			fields.erase(it->first);
		}
		if (!fields.empty()) return false;

		/*
		 *	If we get here the fields are the same. Now run through and
		 *	fail if any of the fields are incompatible
		 */

		for (it = type->object.begin(); it != type->object.end(); ++it) {
			ref<JSDeclTemplate> self = object[it->first];
			if (!self->isCompatible(it->second)) return false;
		}
		return true;
	}

	/* For all other fields, the fact that they're the same is enough */
	return true;
}

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	JSAnalysis::JSAnalysis
 *
 *		Construction
 */

JSAnalysis::JSAnalysis()
{
}

JSAnalysis::~JSAnalysis()
{
}


/************************************************************************/
/*																		*/
/*	Run Analysis														*/
/*																		*/
/************************************************************************/

/*	JSAnalysis::addValue
 *
 *		Add value to the list. This parses through the JSON and updates
 *	the object map
 */

void JSAnalysis::addValue(ref<JSONValue> value)
{
	std::string str = "feed";	/* Name == feed for top */

	ref<JSDeclTemplate> templ = new JSDeclTemplate(value);
	addObject(str,templ);
}

static bool isReservedWord(const std::string &str)
{
	int i = 0;
	char ascii[32];
	if (str.length() > 31) return false;
	for (std::string::const_iterator ptr = str.begin(); ptr != str.end(); ++ptr) {
		ascii[i++] = (char)*ptr;
	}
	ascii[i] = 0;

	if (!strcmp(ascii, "string")) return true;
	if (!strcmp(ascii, "integer")) return true;
	if (!strcmp(ascii, "real")) return true;
	if (!strcmp(ascii, "boolean")) return true;
	if (!strcmp(ascii, "object")) return true;
	if (!strcmp(ascii, "arrayof")) return true;
	return false;
}

/*	JSAnalysis::addObject
 *
 *		Add object declaration for individual objects. This uses the name to
 *	determine the name of the object we're declaring. If the type is
 *	not an object, then we sort through the type to figure out what it is.
 */

void JSAnalysis::addObject(const std::string &name, ref<JSDeclTemplate> templ)
{
	/*
	 *	if we're an array, we're actually only interested in the underlying
	 *	object name
	 */

	while (templ->declType == JSONTYPE_ARRAY) {
		templ = templ->arrayType;
	}

	/*
	 *	If this is not an object we're not interested
	 */

	if (templ->declType != JSONTYPE_OBJECT) return;

	/*
	 *	We now have a JSON object. Add to the list of json objects by name,
	 *	creating a new name if needed because the objects are not compatible.
	 */

	bool fix = false;
	ref<JSDeclTemplate> exist;

	std::string tname = FixUp(name);
	if (isReservedWord(tname)) {
		fix = true;

	} else {
		/*
		 *	Word not reserved. Attempt to load as-is
		 */

		exist = langMap[tname];
		if (exist == NULL) {
			langMap[tname] = new JSDeclTemplate(templ,tname);
			templ->name = tname;

		} else if (exist->isCompatible(templ)) {
			exist->merge(templ);
			templ->name = tname;

		} else {
			fix = true;
		}
	}

	if (fix) {				/* Couldn't write */
		int i = 1;
		for (;;) {
			tname = AppendIndex(name, i++);
			exist = langMap[tname];
			if (exist == NULL) {
				langMap[tname] = new JSDeclTemplate(templ,tname);
				templ->name = tname;
				break;
			} else if (exist->isCompatible(templ)) {
				exist->merge(templ);
				templ->name = tname;
				break;
			}
		}
	}

	/*
	 *	Now iterate through the contents of the object and add each of the
	 *	fields inside that object
	 */

	std::map<std::string, ref<JSDeclTemplate> >::iterator it;
	for (it = templ->object.begin(); it != templ->object.end(); ++it) {
		addObject(it->first, it->second);
	}
}

/************************************************************************/
/*																		*/
/*	Generate															*/
/*																		*/
/************************************************************************/

static void WriteDeclaration(ref<JSOutputStream> out, ref<JSDeclTemplate> templ, int indent)
{
	std::map<std::string, ref<JSDeclTemplate> >::iterator it;

	while (templ->declType == JSONTYPE_ARRAY) {
		out->writeASCIIString("arrayof ");
		templ = templ->arrayType;
	}

	switch (templ->declType) {
		case JSONTYPE_UNDEFINED:
			out->writeASCIIString("undefined");
			break;
		case JSONTYPE_ILLEGAL:
			out->writeASCIIString("illegal");
			break;
		case JSONTYPE_STRING:
			out->writeASCIIString("string");
			break;
		case JSONTYPE_BOOLEAN:
			out->writeASCIIString("boolean");
			break;
		case JSONTYPE_NUMBER:
			out->writeASCIIString("number");
			break;
		case JSONTYPE_OBJECT:
			out->writeASCIIString("{\n");
			for (it = templ->object.begin(); it != templ->object.end(); ++it) {
				for (int i = 0; i <= indent; ++i) out->writeNextChar('\t');
				out->writeString(it->first);
				out->writeASCIIString(": ");
				WriteDeclaration(out, it->second, indent+1);
				out->writeASCIIString(",\n");
			}
			for (int i = 0; i < indent; ++i) out->writeNextChar('\t');
			out->writeASCIIString("}");
			break;
	}
}

/*	JSAnalysis::generate
 *
 *		Generate to the ouptut stream a language representation of the
 *	language map that we've accumulated
 */

void JSAnalysis::generate(ref<JSOutputStream> out)
{
	out->writeASCIIString("/*  Automatically generated output file  */\n\n");

	std::map<std::string, ref<JSDeclTemplate> >::iterator it;
	for (it = langMap.begin(); it != langMap.end(); ++it) {
		ref<JSDeclTemplate> obj = it->second;

		/*
		 *	Variable = object declaration
		 */

		out->writeASCIIString("/*  ");
		out->writeString(it->first);
		out->writeASCIIString("\n *\n *\n */\n\n");

		/*
		 *	Write the header of the declaration
		 */

		out->writeString(it->first);
		out->writeASCIIString(" {");

		bool flag = false;

		std::map<std::string, ref<JSDeclTemplate> >::iterator items;
		for (items = obj->object.begin(); items != obj->object.end(); ++items) {
			if (flag) {
				out->writeASCIIString(",\n");
			} else {
				out->writeASCIIString("\n");
				flag = true;
			}

			/*
			 *	Each row has the format
			 *
			 *		<name> : <decl> with comma separator except for last
			 */

			out->writeASCIIString("\t");
			if (IsValidField(items->first)) {
				out->writeString(items->first);
			} else {
				std::string esc;
				esc = "\"";
				esc.append(EscapeString(items->first));
				esc.push_back('"');

				out->writeString(esc);
			}
			out->writeASCIIString(": ");

			/*
			 *	We now have a declaration we need to generate
			 */

			ref<JSDeclTemplate> decl = items->second;
			if ((decl->isNullable) || (decl->isOptional)) {
				out->writeASCIIString("( ");
				if (decl->isOptional) {
					out->writeASCIIString("optional ");
				}
				if (decl->isNullable) {
					out->writeASCIIString("nullable ");
				}
				out->writeASCIIString(") ");
			}

			while (decl->declType == JSONTYPE_ARRAY) {
				out->writeASCIIString("arrayof ");
				decl = decl->arrayType;

				if (decl->isNullable) {
					out->writeASCIIString("( nullable ) ");
				}
			}

			if (decl->declType == JSONTYPE_OBJECT) {
				out->writeString(decl->name);
			} else if (decl->declType == JSONTYPE_UNDEFINED) {
				out->writeASCIIString("object /* UNDEFINED */");
			} else if (decl->declType == JSONTYPE_ILLEGAL) {
				out->writeASCIIString("object /* INCONSISTENT */\n/*****\n");
				out->writeASCIIString("found the following: \n");
				/* TODO: Write various possible values */
				std::vector<ref<JSDeclTemplate> >::iterator vptr;
				for (vptr = decl->illegalList.begin(); vptr != decl->illegalList.end(); ++vptr) {
					out->writeASCIIString("-\t");
					WriteDeclaration(out, *vptr, 1);
					out->writeASCIIString("\n");
				}
				out->writeASCIIString("*****/");
			} else if (decl->declType == JSONTYPE_NUMBER) {
				if (decl->isReal) {
					out->writeASCIIString("real");
				} else {
					out->writeASCIIString("integer");
				}
			} else if (decl->declType == JSONTYPE_BOOLEAN) {
				out->writeASCIIString("boolean");
			} else if (decl->declType == JSONTYPE_STRING) {
				out->writeASCIIString("string");
			} else {
				out->writeASCIIString("?");
			}
		}

		out->writeASCIIString("\n}\n\n");
	}
}
