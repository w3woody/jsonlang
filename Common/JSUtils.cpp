//
//  JSUtils.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 8/17/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include "JSUtils.h"
#include <string.h>

/************************************************************************/
/*																		*/
/*	Utilities															*/
/*																		*/
/************************************************************************/

std::string AppendIndex(const std::string &str, int index)
{
	char buffer[32];

	sprintf(buffer,"%d",index);
	std::string ret(FixUp(str));
	ret += buffer;

	return ret;
}

/*
 *	If the object is not a valid token value, fix up the string to be
 *	a valid token. This also capitalizes the first character.
 */

std::string FixUp(const std::string &str)
{
	bool cap = true;
	std::string ret;
	size_t i,len = str.size();

	if (len == 0) return "_";		/* Empty string */

	char first = str[0];
	if (!isalpha(first) && (first != '_')) {
		ret.push_back('_');
		cap = false;
	}

	for (i = 0; i < len; ++i) {
		char ch = str[i];
		if (!isalnum(ch)) {
			cap = true;
		} else {
			if (cap) {
				ch = toupper(ch);
				cap = false;
			} else {
				ch = tolower(ch);
			}
			ret.push_back(ch);
		}
	}

	return ret;
}

/*
 *	Determine if this is a valid field. If not, return false.
 */

bool IsValidField(const std::string &str)
{
	size_t i,len = str.size();

	if (len == 0) return false;
	char c = str[0];
	if ((c != '_') && !isalpha(c)) return false;
	for (i = 1; i < len; ++i) {
		if ((c != '_') && !isalnum(c)) return false;
	}
	return true;
}

std::string EscapeString(const std::string &str)
{
	std::string ret;

	size_t i,len = str.size();
	for (i = 0; i < len; ++i) {
		char c = str[i];
		if (c == '\b') ret.append("\\b");
		else if (c == '\b') ret.append("\\b");
		else if (c == '\f') ret.append("\\f");
		else if (c == '\n') ret.append("\\n");
		else if (c == '\r') ret.append("\\r");
		else if (c == '\t') ret.append("\\t");
		else if ((c < 32) || (c > 127)) {
			char buffer[32];
			sprintf(buffer,"\\x%02x",(uint8_t)c);
			ret.append(buffer);
		} else {
			ret.push_back(c);
		}
	}

	return ret;
}