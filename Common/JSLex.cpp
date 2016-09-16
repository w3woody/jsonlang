//
//  JSLex.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 8/15/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include "JSLex.h"
#include "JSUtils.h"
#include <ctype.h>

/************************************************************************/
/*																		*/
/*	Internal Structures													*/
/*																		*/
/************************************************************************/

struct JSLexInternal
{
	std::string match;
	uint32_t val;
};

/************************************************************************/
/*																		*/
/*	Utilities															*/
/*																		*/
/************************************************************************/

static JSLexInternal *Find(JSLexInternal *array, int max, std::string &str)
{
	for (int i = 0; i < max; ++i) {
		JSLexInternal *ptr = array + i;
		if (ptr->match == str) return ptr;
	}
	return NULL;
}


static int FromXDigit(int32_t ch)
{
	if ((ch >= '0') && (ch <= '9')) return ch - '0';
	if ((ch >= 'a') && (ch <= 'f')) return ch - 'a' + 10;
	if ((ch >= 'A') && (ch <= 'F')) return ch - 'A' + 10;
	return 0;
}

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	JSLex::JSLex
 *
 *		Construct this with quick lookup of strings
 */

JSLex::JSLex(ref<JSInputStream> i, const char * const reserved[])
{
	sc = new JSStripComment(i);
	input = new JSBufferedInputStream(sc.get());

	pushFlag = false;
	lastRead = 0;

	offset = 0;
	length = 256;
	text = new char[length];

	int ct = 0;
	for (const char * const *ptr = reserved; *ptr; ptr++) {
		++ct;
	}
	
	max = ct;
	keywords = new JSLexInternal[ct];
	for (int i = 0; i < ct; ++i) {
		keywords[i].match = reserved[i];
		keywords[i].val = SYMBOL_RESERVED + i;
	}
}

JSLex::~JSLex()
{
	delete[] keywords;
	delete[] text;
}

/************************************************************************/
/*																		*/
/*	Support																*/
/*																		*/
/************************************************************************/

/*	JSLex::append
 *
 *		Append to internal buffer, resizing as needed
 */

void JSLex::append(uint16_t ch)
{
	if (offset >= length) {
		int newsize = (length * 4) / 3;
		char *newPtr = new char[newsize];
		memmove(newPtr, text, offset * sizeof(uint16_t));

		char *oldPtr = text;
		text = newPtr;
		delete[] oldPtr;

		length = newsize;
	}

	text[offset++] = ch;
}

/************************************************************************/
/*																		*/
/*	Tokenization														*/
/*																		*/
/************************************************************************/

/*	JSLex::nextToken
 *
 *		Obtain the next token. Uses tokenization rules for JSON, but
 *	without looking for double character operators
 */

int32_t JSLex::nextToken()
{
	if (pushFlag) {
		pushFlag = false;
		return lastRead;
	}

	int32_t c;

	do {
		c = input->readNextChar();
	} while (isspace(c));
	offset = 0;

	lastLine = sc->getCurLine();	/* Get current line in underlying buffer */
	if (c == -1) return lastRead = -1;		/* EOF */

	/*
	 *	Test for token
	 */

	if (isalpha(c) || (c == '_') || (c == '$')) {
		while (isalnum(c) || (c == '_') || (c == '$')) {
			append(c);
			c = input->readNextChar();
		}
		input->pushBackChar(c);

		/*
		 *	We have a token. Now determine if it is a reserved keyword
		 */

		std::string str(text,offset);
		JSLexInternal *k = Find(keywords,max,str);

		if (k) {
			return lastRead = k->val;
		} else {
			return lastRead = SYMBOL_TOKEN;
		}
	}

	/*
	 *	Test for string
	 */

	if (c == '"') {
		sc->disableStripComment();

		/*
		 *	This will also convert a string containing string escapes
		 */

		for (;;) {
			c = input->readNextChar();

			if (c == '"') break;
			if (c == -1) return lastRead = -1;
			if (c == '\\') {
				c = input->readNextChar();
				if (c == 'b') {
					append('\b');
				} else if (c == 'f') {
					append('\f');
				} else if (c == 'n') {
					append('\n');
				} else if (c == 'r') {
					append('\r');
				} else if (c == 't') {
					append('\t');
				} else if (c == 'u') {
					/*
					 *	4 hex characters
					 */

					uint16_t tmp = 0;
					for (int i = 0; i < 4; ++i) {
						c = input->readNextChar();
						if (isxdigit(c)) {
							tmp = (tmp << 4) | FromXDigit(c);
						} else {
							input->pushBackChar(c);
							break;
						}
					}
					append(tmp);
				} else {
					append(c);
				}
			} else {
				append(c);
			}
		}

		sc->enableStripComment();

		return lastRead = SYMBOL_STRING;
	}

	/*
	 *	Test for numeric
	 */

	bool number = false;
	if (isdigit(c)) number = true;
	if (c == '-') {
		append(c);
		c = input->readNextChar();
		if (isdigit(c)) number = true;
		else {
			input->pushBackChar(c);
			return lastRead = '-';
		}
	}

	if (number) {
		bool real = false;
		if (c != '0') {
			while (isdigit(c)) {
				append(c);
				c = input->readNextChar();
			}
		} else {
			append(c);
			c = input->readNextChar();
		}

		if (c == '.') {
			append(c);
			real = true;
			for (;;) {
				c = input->readNextChar();
				if (isdigit(c)) append(c);
				else break;
			}
		}

		if ((c == 'e') || (c == 'E')) {
			uint32_t d = input->readNextChar();
			if ((d == '+') || (d == '-')) {
				uint32_t e = input->readNextChar();
				if (isdigit(e)) {
					real = true;
					append(c);
					append(d);
					c = e;
					while (isdigit(c)) {
						append(c);
						c = input->readNextChar();
					}
				} else {
					input->pushBackChar(e);
					input->pushBackChar(d);
				}
			} else if (isdigit(d)) {
				real = true;
				append(c);
				c = d;
				while (isdigit(c)) {
					append(c);
					c = input->readNextChar();
				}
			} else {
				input->pushBackChar(d);
			}
		}

		input->pushBackChar(c);

		return lastRead = real ? SYMBOL_REAL : SYMBOL_INTEGER;
	}

	/*
	 *	Naked character. Simply return
	 */

	append(c);
	return lastRead = c;
}
