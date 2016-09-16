//
//  JSGenObjectiveC.h
//  AnalyzeJSON
//
//  Created by William Woody on 8/17/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#ifndef __AnalyzeJSON__JSGenObjectiveC__
#define __AnalyzeJSON__JSGenObjectiveC__

#include <iostream>
#include "JSObject.h"
#include "JSParser.h"

/************************************************************************/
/*																		*/
/*	Objective C Generation												*/
/*																		*/
/************************************************************************/

/*	JSGenObjectiveC
 *
 *		Generate Objective C code from prefix, specifications
 */

class JSGenObjectiveC : public JSObject
{
	public:
						JSGenObjectiveC(std::string path, std::string prefix, bool ro, ref<JSParser> parser);
						~JSGenObjectiveC();

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
		std::string		genObjC(std::string name);

		void			indent(FILE *out, int indent);
		void			genArrayLoader(FILE *out, ref<JSDeclType> type, int aix, int indent);
		bool			genArraySave(FILE *out, ref<JSDeclType> type, int indent, std::string destVar, std::string srcVar, int aix);

		std::string		genTempVar(const char *prefix, int index);
		std::string		genPath(std::string objName);
		void			genHeader(std::string objName, ref<JSParserObject> obj);
		void			genSource(std::string objName, ref<JSParserObject> obj);

};

#endif /* defined(__AnalyzeJSON__JSGenObjectiveC__) */
