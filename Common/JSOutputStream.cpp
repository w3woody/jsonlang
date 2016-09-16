//
//  JSOutputStream.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 8/15/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include "JSOutputStream.h"
#include <string.h>
#include <string>
#include "JSUtils.h"

/************************************************************************/
/*																		*/
/*	Support																*/
/*																		*/
/************************************************************************/

/*	JSOutputStream::writeData
 *
 *		Write data
 */

void JSOutputStream::writeData(size_t ct, const char *data)
{
	for (size_t i = 0; i < ct; ++i) {
		writeNextChar(data[i]);
	}
}

/*	JSOutputStream::writeASCIIString
 *
 *		Write constant string. Note that this does not process UTF-8
 *	characters
 */

void JSOutputStream::writeASCIIString(const char *str)
{
	writeData(strlen(str),str);
}

/*	JSOutputStream::writeString
 *
 *		Write constant string. Note that this does not process UTF-8
 *	characters
 */

void JSOutputStream::writeString(const std::string &str)
{
	std::string::const_iterator it;
	for (it = str.begin(); it != str.end(); ++it) {
		writeNextChar(*it);
	}
}
