//
//  JSFileInputStream.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 8/15/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include "JSInputStream.h"

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	JSFileInputStream::JSFileInputStream
 *
 *		Construct me with the file. This will take ownershp of the file
 *	and close it when the file is done
 */

JSFileInputStream::JSFileInputStream(FILE *f)
{
	file = f;
}

/*	JSFileInputStream::~JSFileInputStream
 *
 *		Destroy me
 */

JSFileInputStream::~JSFileInputStream()
{
	fclose(file);
}

/************************************************************************/
/*																		*/
/*	Reader																*/
/*																		*/
/************************************************************************/

int JSFileInputStream::readNextChar()
{
	return fgetc(file);
}
