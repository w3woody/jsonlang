//
//  JSBufferedInputStream.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 8/15/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include "JSInputStream.h"
#include <string.h>

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	JSBufferedInputStream::JSBufferedInputStream
 *
 *		Construct me
 */

JSBufferedInputStream::JSBufferedInputStream(ref<JSInputStream> is) : input(is)
{
	index = 0;
	memset(buffer,0,sizeof(buffer));
}

/*	JSBufferedInputStream::~JSBufferedInputStream
 *
 *		Delete me
 */

JSBufferedInputStream::~JSBufferedInputStream()
{
}

/************************************************************************/
/*																		*/
/*	Methods																*/
/*																		*/
/************************************************************************/

/*	JSBufferedInputStream::readNextChar
 *
 *		Read the next character from the underlying buffer
 */

int JSBufferedInputStream::readNextChar()
{
	int32_t ch;

	if (index > 0) {
		ch = buffer[--index];
	} else {
		ch = input->readNextChar();
	}

	if (ch == '\n') ++curLine;
	return ch;
}

/*	JSBufferedInputStream::pushBackChar
 *
 *		Push back onto stack
 */

void JSBufferedInputStream::pushBackChar(int ch)
{
	if (index < MAXPUSHBACK) {
		buffer[index++] = ch;
	}

	if (ch == '\n') --curLine;
}
