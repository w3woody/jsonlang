//
//  JSAnalysis.h
//  AnalyzeJSON
//
//  Created by William Woody on 8/16/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#ifndef __AnalyzeJSON__JSAnalysis__
#define __AnalyzeJSON__JSAnalysis__

#include <iostream>
#include <map>
#include "JSOutputStream.h"
#include "JSJSONParser.h"

/************************************************************************/
/*																		*/
/*	Analysis Object														*/
/*																		*/
/************************************************************************/

/*
 *	Note: JSDeclTemplate uses the JSONTYPE constants to define the type
 *	of this object, alongw ith an additional 'undefed' object when something
 *	is undefined
 */

#define JSONTYPE_UNDEFINED	0
#define JSONTYPE_ILLEGAL	-1

/*	JSDeclTemplate
 *
 *		Declaration template
 */

class JSDeclTemplate : public JSObject
{
	public:
						JSDeclTemplate()
							{
								declType = JSONTYPE_UNDEFINED;
								isNullable = false;
								isOptional = false;
							}

						JSDeclTemplate(ref<JSONValue> ref);
						JSDeclTemplate(ref<JSDeclTemplate> ref, const std::string &name);

		void			merge(ref<JSDeclTemplate> type);

		/* Used to determine if two JSObject declarations are compatible.
		 * This is true if both fields have the same set of common fields.
		 *
		 * This will be used to determine if two objects with the same name
		 * are compatible with each other and can be collapsed into a single
		 * object.
		 */
		bool			isCompatible(ref<JSDeclTemplate> type);

		int				declType;
		bool			isNullable;
		bool			isOptional;
		bool			isReal;				/* Is real value */
		std::string		name;				/* Name of object after object added */

		ref<JSDeclTemplate>	arrayType;		/* Defined only if array type */
		std::map<std::string, ref<JSDeclTemplate> > object;
		std::vector<ref<JSDeclTemplate> > illegalList;
};

/************************************************************************/
/*																		*/
/*	JSON Analysis														*/
/*																		*/
/************************************************************************/

/*	JSAnalysis
 *
 *		Given a value object, this maintains a map of all of the objects
 *	and their declaration.
 */

class JSAnalysis : public JSObject
{
	public:
						JSAnalysis();
						~JSAnalysis();

		void			addValue(ref<JSONValue> value);
		void			generate(ref<JSOutputStream> os);

	private:
		void			addObject(const std::string &name, ref<JSDeclTemplate> templ);

		std::map<std::string,ref<JSDeclTemplate> > langMap;
};


#endif /* defined(__AnalyzeJSON__JSAnalysis__) */
