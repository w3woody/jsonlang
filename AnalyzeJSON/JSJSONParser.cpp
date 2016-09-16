//
//  JSJSONParser.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 8/16/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include "JSJSONParser.h"

#define SYMBOL_TRUE			SYMBOL_RESERVED
#define SYMBOL_FALSE		(SYMBOL_RESERVED + 1)
#define SYMBOL_NULL			(SYMBOL_RESERVED + 2)

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	JSJSONParser::JSJSONParser
 *
 *		Construct the parsesr for parsing JSON
 */

JSJSONParser::JSJSONParser(ref<JSInputStream> is)
{
	const char * const keywords[] = { "true", "false", "null", NULL };
	lex = new JSLex(is,keywords);
}

/*	JSJSONParser::~JSJSONParser
 *
 *		Delete me
 */

JSJSONParser::~JSJSONParser()
{
}

/************************************************************************/
/*																		*/
/*	Internal Support													*/
/*																		*/
/************************************************************************/

/*	JSJSONParser::parseObject
 *
 *		This is called after we've read the '{', and is used to parse
 *	the balance of the object. This will return with the next token being
 *	the one past the close '}'
 */

ref<JSONValue> JSJSONParser::parseObject()
{
	JSONObject *obj = new JSONObject;
	int32_t token;

	token = lex->nextToken();
	if (token == '}') return obj;	/* Empty object */
	lex->pushToken();

	for (;;) {
		token = lex->nextToken();

		if (token != SYMBOL_STRING) {
			throw JSException(lex->getLine(),"Syntax error: expected key as string");
		}
		std::string key = lex->getTokenValue();

		token = lex->nextToken();
		if (token != ':') {
			throw JSException(lex->getLine(),"Syntax error: expected ':'");
		}

		ref<JSONValue> value = parse();
		(*obj)[key] = value;

		token = lex->nextToken();
		if (token == '}') return obj;
		if (token != ',') {
			throw JSException(lex->getLine(),"Syntax error: expected ','");
		}
	}
}

/*	JSJSONParser::parseArray
 *
 *		Parse array of objects
 */

ref<JSONValue> JSJSONParser::parseArray()
{
	JSONArray *obj = new JSONArray;
	int32_t token;

	token = lex->nextToken();
	if (token == ']') return obj;	/* Empty array */
	lex->pushToken();

	for (;;) {
		ref<JSONValue> value = parse();
		obj->push_back(value);

		token = lex->nextToken();
		if (token == ']') return obj;	/* End of array */
		if (token != ',') {
			throw JSException(lex->getLine(),"Syntax error: expected comma in array");
		}
	}

	return obj;
}

/*	JSJSONParser::parse
 *
 *		Parse a top level object
 */

ref<JSONValue> JSJSONParser::parse()
{
	int32_t token;

	token = lex->nextToken();
	if (token == -1) return NULL;

	if (token == '{') return parseObject();
	if (token == '[') return parseArray();

	if ((token == SYMBOL_TRUE) || (token == SYMBOL_FALSE)) {
		return new JSONBoolean(token == SYMBOL_TRUE);
	}

	if (token == SYMBOL_NULL) {
		return new JSONNull;
	}

	if (token == SYMBOL_STRING) {
		return new JSONString(lex->getTokenValue());
	}

	if ((token == SYMBOL_REAL) || (token == SYMBOL_INTEGER)) {
		return new JSONNumber(lex->getTokenValue());
	}

	throw JSException(lex->getLine(),"Syntax error: unexpected token");
}
