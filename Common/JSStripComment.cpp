//
//  JSFileOutputStream.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 8/15/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include "JSLex.h"

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	JSStripComment::JSStripComment
 *
 *		Construction/destruction
 */

JSStripComment::JSStripComment(ref<JSInputStream> is)
{
	input = new JSBufferedInputStream(is);
	stripFlag = true;
}

/*	JSStripComment::~JSStripComment
 *
 *		Delete
 */

JSStripComment::~JSStripComment()
{
}

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	JSStripComment::readNextChar
 *
 *		Read next character
 */

int JSStripComment::readNextChar()
{
	int ch;

	ch = input->readNextChar();
	if (!stripFlag) return ch;
	if (ch == -1) return ch;
	if (ch != '/') return ch;

	ch = input->readNextChar();
	if (ch == '/') {
		/*
		 *	Run forward until '\n', '\r' or -1
		 */

		for (;;) {
			ch = input->readNextChar();
			if ((ch == '\n') || (ch == '\r') || (ch == -1)) return ch;
		}
	} else if (ch == '*') {
		/*
		 *	Read forward until '*' and '/'
		 */

		bool skipRead = false;
		for (;;) {
			if (!skipRead) {
				ch = input->readNextChar();
			} else {
				skipRead = false;
			}

			if (ch == '*') {
				ch = input->readNextChar();
				if (ch == -1) return -1;
				if (ch == '/') return ' ';
				skipRead = true;
			} else if (ch == -1) {
				return -1;
			}
		}
	} else {
		/*
		 *	Not a comment
		 */

		input->pushBackChar(ch);
		return '/';
	}
}

