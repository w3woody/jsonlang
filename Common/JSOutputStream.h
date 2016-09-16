//
//  JSOutputStream.h
//  AnalyzeJSON
//
//  Created by William Woody on 8/15/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#ifndef AnalyzeJSON_JSOutputStream_h
#define AnalyzeJSON_JSOutputStream_h

#include <stdint.h>
#include <stdio.h>
#include <string>
#include "JSObject.h"

/************************************************************************/
/*																		*/
/*	Constants															*/
/*																		*/
/************************************************************************/

#define MAXPUSHBACK		16

/************************************************************************/
/*																		*/
/*	Output Stream														*/
/*																		*/
/************************************************************************/

/*	JSOutputStream
 *
 *		Output stream abstraction.
 */

class JSOutputStream : public JSObject
{
	public:
		virtual void writeNextChar(char ch) = 0;

		virtual void writeData(size_t ct, const char *data);
		virtual void writeString(const std::string &str);
		virtual void writeASCIIString(const char *str);
};

/*	JSFileOutputStream
 *
 *		Basic file writer
 */

class JSFileOutputStream : public JSOutputStream
{
	public:
							JSFileOutputStream(FILE *f);
		virtual				~JSFileOutputStream();

		void				writeNextChar(char ch);

	private:
		FILE				*file;
};


#endif
