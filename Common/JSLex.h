//
//  JSLex.h
//  AnalyzeJSON
//
//  Created by William Woody on 8/15/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#ifndef __AnalyzeJSON__JSLex__
#define __AnalyzeJSON__JSLex__

#include "JSInputStream.h"
#include <string>

/************************************************************************/
/*																		*/
/*	Preprocessor														*/
/*																		*/
/************************************************************************/

/*	JSStripComment
 *
 *		Input stream which strips comments out
 */

class JSStripComment : public JSInputStream
{
	public:
					JSStripComment(ref<JSInputStream> is);
					~JSStripComment();

		int			readNextChar();

		void		enableStripComment()
						{
							stripFlag = true;
						}

		void		disableStripComment()
						{
							stripFlag = false;
						}

		int			getCurLine()
						{
							return input->getCurLine();
						}

	private:
		bool		stripFlag;
		ref<JSBufferedInputStream> input;
};

/************************************************************************/
/*																		*/
/*	Lexical Parser														*/
/*																		*/
/************************************************************************/

/*
 *	Predefined tokens
 */

#define SYMBOL_TOKEN	0x10000
#define SYMBOL_STRING	0x10001
#define SYMBOL_INTEGER	0x10002
#define SYMBOL_REAL		0x10003
#define SYMBOL_RESERVED	0x10004		/* First reserved word token */

/*	JSLex
 *
 *		This takes an input stream and pulls apart the file into separate
 *	tokens. 
 */

class JSLex : public JSObject
{
	public:
					JSLex(ref<JSInputStream> input, const char * const reserved[]);
					~JSLex();

		int32_t		nextToken();
		void		pushToken()
						{
							pushFlag = true;
						}

		int32_t		getLine()
						{
							return lastLine+1;
						}

		std::string getTokenValue()
						{
							return std::string(text,offset);
						}

	private:
		ref<JSStripComment> sc;
		ref<JSBufferedInputStream> input;

		bool		pushFlag;
		int32_t		lastRead;
		int16_t		lastLine;

		int			offset;
		int			length;
		char		*text;

		void		append(uint16_t ch);

		int			max;
		struct JSLexInternal *keywords;
};


#endif /* defined(__AnalyzeJSON__JSLex__) */
