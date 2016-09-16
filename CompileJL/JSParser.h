//
//  JSParser.h
//  AnalyzeJSON
//
//  Created by William Woody on 8/17/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#ifndef __AnalyzeJSON__JSParser__
#define __AnalyzeJSON__JSParser__

#include <iostream>
#include <map>
#include <vector>
#include "JSInputStream.h"
#include "JSObject.h"
#include "JSLex.h"

/************************************************************************/
/*																		*/
/*	Language State														*/
/*																		*/
/************************************************************************/

#define TYPE_INTEGER		1
#define TYPE_REAL			2
#define TYPE_BOOLEAN		3
#define TYPE_STRING			4
#define TYPE_OBJECT			5
#define TYPE_GENOBJECT		6
#define TYPE_ARRAY			7

class JSDeclType : public JSObject
{
	public:
		bool		isNullable;
		bool		isOptional;
		bool		isArray;

		int			type;			/* Symbol/integer/real/boolean/object or 0 */

		std::string symbol;
		ref<JSDeclType> arrayType;
};

class JSParserObject : public JSObject, public std::map<std::string,ref<JSDeclType> >
{
};

/************************************************************************/
/*																		*/
/*	Language Parser														*/
/*																		*/
/************************************************************************/

class JSParser : public JSObject
{
	public:
						JSParser(ref<JSInputStream> is);
						~JSParser();

		bool			run();

		std::map<std::string, ref<JSParserObject> >	data;
	private:
		ref<JSLex>		lex;

		bool			error;

		void			parseSymbolDecl(std::string token);
		ref<JSDeclType>	parseDecl();

};

#endif /* defined(__AnalyzeJSON__JSParser__) */
