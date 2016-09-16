//
//  JSGenJava.h
//  AnalyzeJSON
//
//  Created by William Woody on 8/24/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#ifndef __AnalyzeJSON__JSGenJava__
#define __AnalyzeJSON__JSGenJava__

#include <iostream>
#include "JSObject.h"
#include "JSParser.h"

/************************************************************************/
/*																		*/
/*	Objective C Generation												*/
/*																		*/
/************************************************************************/

/*	JSGenJava
 *
 *		Generate Java code from prefix, specifications
 */

class JSGenJava : public JSObject
{
	public:
						JSGenJava(std::string path, std::string prefix, bool ro, ref<JSParser> parser);
						~JSGenJava();

		void			run();

	private:
		ref<JSParser>	parser;
		std::string		path;
		std::string		prefix;
		bool			readOnly;

		std::string		curtime;

		std::map<std::string,std::string> mapNames;

		std::string		genUnique(std::string name);
		std::string		genName(std::string name);
		std::string		genCapName(std::string name);
		std::string		genJava(std::string name);
		std::string		genTempVar(const char *prefix, int index);

		void			indent(FILE *out, int indent);

		std::string		varType(ref<JSDeclType> type, bool subType = false);

		void			genArrayLoader(FILE *out, std::string destVar, ref<JSDeclType> srcType, std::string srcVar, int aix, int indent);
		void			genArraySave(FILE *out, std::string destVar, ref<JSDeclType> srcType, std::string srcVar, int aix, int indent);

		std::string		genPath(std::string objName);
		void			genSource(std::string objName, ref<JSParserObject> obj);
};

#endif /* defined(__AnalyzeJSON__JSGenJava__) */
