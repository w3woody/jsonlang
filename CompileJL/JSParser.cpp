//
//  JSParser.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 8/17/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include "JSParser.h"
#include "JSLex.h"
#include <stdio.h>

#define KEYWORD_STRING		SYMBOL_RESERVED
#define KEYWORD_INTEGER		SYMBOL_RESERVED + 1
#define KEYWORD_REAL		SYMBOL_RESERVED + 2
#define KEYWORD_BOOLEAN		SYMBOL_RESERVED + 3
#define KEYWORD_OBJECT		SYMBOL_RESERVED + 4
#define KEYWORD_ARRAYOF		SYMBOL_RESERVED + 5
#define KEYWORD_OPTIONAL	SYMBOL_RESERVED + 6
#define KEYWORD_NULLABLE	SYMBOL_RESERVED + 7

/************************************************************************/
/*																		*/
/*	Construction														*/
/*																		*/
/************************************************************************/

/*	JSParser::JSParser
 *
 *		Build the parser 
 */

JSParser::JSParser(ref<JSInputStream> is)
{
	const char * const reserved[] = { "string",
									  "integer",
									  "real",
									  "boolean",
									  "object",
									  "arrayof",
									  "optional",
									  "nullable" };
	lex = new JSLex(is,reserved);
}

JSParser::~JSParser()
{
}

static int32_t WindForward(ref<JSLex> lex, int ch = '}')
{
	int32_t t;

	while (-1 != (t = lex->nextToken())) {
		if (t == ch) break;
	}

	return t;
}

/*	JSParser::run
 *
 *		Run the top level parser. This prints error messages as they are
 *	encountered, but attempts to run as long as possible
 */

bool JSParser::run()
{
	int32_t t;
	error = false;

	while (-1 != (t = lex->nextToken())) {
		if ((t == SYMBOL_TOKEN) || (t >= SYMBOL_RESERVED)) {
			/*
			 *	start of symbol declaration. Parse the rest
			 */

			std::string token = lex->getTokenValue();
			parseSymbolDecl(token);
		} else {
			/*
			 *	Unexpected symbol; skip forward
			 */

			fprintf(stderr,"Line %d: Illegal symbol %s\n",lex->getLine(),lex->getTokenValue().c_str());
			error = true;
			WindForward(lex);
		}
	}

	/*
	 *	TODO: Run through all the declarations and make sure they're all
	 *	defined
	 */

	std::map<std::string, ref<JSParserObject> >::iterator declIter;

	for (declIter = data.begin(); declIter != data.end(); ++declIter) {
		std::map<std::string,ref<JSDeclType> >::iterator decl;
		ref<JSParserObject> obj = declIter->second;

		for (decl = obj->begin(); decl != obj->end(); ++decl) {
			ref<JSDeclType> ref = decl->second;

			while (ref->isArray) {
				ref = ref->arrayType;
			}
			if (ref->type == TYPE_OBJECT) {
				if (!data.count(ref->symbol)) {
					fprintf(stderr,"Symbol %s is undefined\n",ref->symbol.c_str());
					error = true;
				}
			}
		}
	}

	return !error;
}

/*
 *	Parse the declaration type. This parses 'decl' in the language spec
 */

ref<JSDeclType>	JSParser::parseDecl()
{
	ref<JSDeclType> type = new JSDeclType;
	int32_t t = lex->nextToken();

	if (t == '(') {
		/*
		 *	Parse the modifiers
		 */

		for (;;) {
			t = lex->nextToken();
			if (t == ')') break;
			if (t == KEYWORD_NULLABLE) {
				type->isNullable = true;
			} else if (t == KEYWORD_OPTIONAL) {
				type->isOptional = true;
			} else {
				fprintf(stderr,"Line %d: expected nullable or optional\n",lex->getLine());
				error = true;
				return NULL;
			}
		}
		t = lex->nextToken();
	}

	if (t == KEYWORD_ARRAYOF) {
		type->isArray = true;
		type->type = TYPE_ARRAY;
		type->arrayType = parseDecl();
		return type;
	}

	if ((t == KEYWORD_STRING) || (t == KEYWORD_INTEGER) || (t == KEYWORD_REAL) || (t == KEYWORD_BOOLEAN) || (t == KEYWORD_OBJECT)) {
		switch (t) {
			case KEYWORD_STRING:
				type->type = TYPE_STRING;
				break;
			case KEYWORD_INTEGER:
				type->type = TYPE_INTEGER;
				break;
			case KEYWORD_REAL:
				type->type = TYPE_REAL;
				break;
			case KEYWORD_BOOLEAN:
				type->type = TYPE_BOOLEAN;
				break;
			case KEYWORD_OBJECT:
				type->type = TYPE_GENOBJECT;
				break;
		}
		return type;
	}

	if (t == SYMBOL_TOKEN) {
		type->type = TYPE_OBJECT;
		type->symbol = lex->getTokenValue();
		return type;
	}

	fprintf(stderr,"Line %d: illegal declaration type %s\n",lex->getLine(),lex->getTokenValue().c_str());
	error = true;
	return NULL;
}


/*	parseSymbolDecl
 *
 *		We have the token. look for '{', then parse the key/values
 */

void JSParser::parseSymbolDecl(std::string token)
{
	int32_t t = lex->nextToken();
	ref<JSParserObject> obj = new JSParserObject;

	if (t == ':') {
		/*
		 *	Parent declarations; merge the token declarations
		 */

		for (;;) {
			t = lex->nextToken();
			if ((t == SYMBOL_TOKEN) || (t >= SYMBOL_RESERVED)) {
				/*
				 *	Look for symbol
				 */

				std::string tk = lex->getTokenValue();
				ref<JSParserObject> decl = data[tk];
				if (decl == NULL) {
					fprintf(stderr,"Line %d: parent symbol %s must be declared prior to use\n",lex->getLine(),tk.c_str());
					t = WindForward(lex,'{');

					break;
				} else {
					/* Merge with check */
					std::map<std::string,ref<JSDeclType> >::iterator iter;
					for (iter = decl->begin(); iter != decl->end(); iter++) {
						if (obj->count(iter->first)) {
							fprintf(stderr,"Line %d: token duplicate declaration in parents\n",lex->getLine());
							t = WindForward(lex,'{');
							break;
						} else {
							(*obj)[iter->first] = iter->second;
						}
					}
				}
			} else {
				fprintf(stderr,"Line %d: Syntax error on parent list\n",lex->getLine());
				t = WindForward(lex,'{');
				break;
			}

			t = lex->nextToken();
			if (t == '{') break;
			if (t != ',') {
				fprintf(stderr,"Line %d: Syntax error; expected comma\n",lex->getLine());
				t = WindForward(lex,'{');
				break;
			}
		}
	}

	if (t != '{') {
		fprintf(stderr,"Line %d: Expected '{' for object declaration\n",lex->getLine());
		error = true;
		WindForward(lex);
		return;
	}

	t = lex->nextToken();
	if (t == '}') {
		/* Empty symbol */
		data[token] = obj;
		return;
	} else {
		lex->pushToken();
	}

	for (;;) {
		t = lex->nextToken();
		if ((t != SYMBOL_TOKEN) && (t != SYMBOL_STRING) && (t < SYMBOL_RESERVED)) {
			fprintf(stderr,"Line %d: Expected object declaration field, not %s\n",lex->getLine(),lex->getTokenValue().c_str());
			error = true;
			WindForward(lex);
			return;
		}

		std::string fieldName = lex->getTokenValue();
		t = lex->nextToken();
		if (t != ':') {
			fprintf(stderr,"Line %d: Expected colon after field name\n",lex->getLine());
			error = true;
			WindForward(lex);
			return;
		}

		ref<JSDeclType> type = parseDecl();
		if (type != NULL) {
			if (obj->count(fieldName) != 0) {
				fprintf(stderr,"Line %d: duplicate field declaration: %s\n",lex->getLine(),fieldName.c_str());
			}
			(*obj)[fieldName] = type;
		}

		t = lex->nextToken();
		if (t == '}') break;
		if (t != ',') {
			fprintf(stderr,"Line %d: Expected comma separator\n",lex->getLine());
			error = true;
			WindForward(lex);
			return;
		}
	}

	data[token] = obj;
	return;
}
