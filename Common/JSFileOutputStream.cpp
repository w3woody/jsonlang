//
//  JSFileOutputStream.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 8/15/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include "JSOutputStream.h"

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	JSFileOutputStream::JSFileOutputStream
 *
 *		Construction/destruction
 */

JSFileOutputStream::JSFileOutputStream(FILE *f)
{
	file = f;
}

/*	JSFileOutputStream::~JSFileOutputStream
 *
 *		Delete
 */

JSFileOutputStream::~JSFileOutputStream()
{
	fclose(file);
}

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	JSFileOutputStream::writeNextChar
 *
 *		Write next character
 */

void JSFileOutputStream::writeNextChar(char ch)
{
	fputc(ch,file);
}

