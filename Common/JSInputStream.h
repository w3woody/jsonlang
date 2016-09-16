//
//  JSInputStream.h
//  AnalyzeJSON
//
//  Created by William Woody on 8/15/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#ifndef AnalyzeJSON_JSInputStream_h
#define AnalyzeJSON_JSInputStream_h

#include <stdint.h>
#include <stdio.h>
#include "JSObject.h"

/************************************************************************/
/*																		*/
/*	Constants															*/
/*																		*/
/************************************************************************/

#define MAXPUSHBACK		16

/************************************************************************/
/*																		*/
/*	Input Stream														*/
/*																		*/
/************************************************************************/

/*	JSInputStream
 *
 *		Input stream abstraction.
 */

class JSInputStream : public JSObject
{
	public:
		virtual int32_t	readNextChar() = 0;	/* >= 0: UNICODE. -1: EOF */
};

/*	JSBufferedInputStream
 *
 *		Handle a push back buffer. Wraps an input stream to provide push
 *	back functionality
 */

class JSBufferedInputStream : public JSInputStream
{
	public:
							JSBufferedInputStream(ref<JSInputStream> is);
							~JSBufferedInputStream();

		int					readNextChar();
		void				pushBackChar(int32_t ch);

		int					getCurLine()
								{
									return curLine;
								}

	private:
		int32_t				curLine;

		uint16_t			index;
		int					buffer[MAXPUSHBACK];
		ref<JSInputStream>	input;
};

/*	JSFileInputStream
 *
 *		Basic file reader
 */

class JSFileInputStream : public JSInputStream
{
	public:
							JSFileInputStream(FILE *f);
		virtual				~JSFileInputStream();

		virtual int			readNextChar();

	private:
		int32_t				readUTFBytes(int start, int num);
		FILE				*file;
};

#endif
