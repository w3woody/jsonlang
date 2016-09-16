//
//  JSJSONParser.h
//  AnalyzeJSON
//
//  Created by William Woody on 8/16/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#ifndef __AnalyzeJSON__JSJSONParser__
#define __AnalyzeJSON__JSJSONParser__

#include <iostream>
#include <vector>
#include <exception>
#include <map>
#include "JSInputStream.h"
#include "JSObject.h"
#include "JSLex.h"

/************************************************************************/
/*																		*/
/*	JSON Values															*/
/*																		*/
/************************************************************************/

#define JSONTYPE_ARRAY		1
#define JSONTYPE_OBJECT		2
#define JSONTYPE_BOOLEAN	3
#define JSONTYPE_STRING		4
#define JSONTYPE_NUMBER		5
#define JSONTYPE_NULL		6

/*	JSONValue
 *
 *		An abstract value in JSON
 */

class JSONValue : public JSObject
{
	public:
		virtual int		getJSONType() = 0;
};

/*	JSONArray
 *
 *		An array type
 */

class JSONArray : public JSONValue, public std::vector<ref<JSONValue> >
{
	public:
		int				getJSONType()
							{
								return JSONTYPE_ARRAY;
							}
};

/*	JSONObject
 *
 *		An object
 */

class JSONObject : public JSONValue, public std::map<std::string,ref<JSONValue> >
{
	public:
		int				getJSONType()
							{
								return JSONTYPE_OBJECT;
							}
};

/*	JSONBoolean
 *
 *		A boolean value
 */

class JSONBoolean : public JSONValue
{
	public:
						JSONBoolean()
							{
								value = false;
							}
						JSONBoolean(bool v)
							{
								value = v;
							}

		JSONBoolean &operator = (bool v)
							{
								value = v;
								return *this;
							}
		JSONBoolean &operator = (const JSONBoolean &v)
							{
								value = (bool)v;
								return *this;
							}

						operator bool() const
							{
								return value;
							}

		int				getJSONType()
							{
								return JSONTYPE_BOOLEAN;
							}

	private:
		bool			value;
};

/*	JSONString
 *
 *		A JSON string
 */

class JSONString: public JSONValue, public std::string
{
	public:
						JSONString()
							{
							}

						JSONString(const std::string &str) : std::string(str)
							{
							}

		int				getJSONType()
							{
								return JSONTYPE_STRING;
							}
};

/*	JSONNull
 *
 *		A JSON null object
 */

class JSONNull: public JSONValue
{
	public:
		int				getJSONType()
							{
								return JSONTYPE_NULL;
							}
};

/*	JSONNumber
 *
 *		A numeric value. Note that we store this as a string for
 *	the time being. The presumption is that when we deserialize this
 *	into containers, we will convert the value to a numeric value at that
 *	point, preserving accuracy
 */

class JSONNumber: public JSONValue, public std::string
{
	public:
						JSONNumber()
							{
							}

						JSONNumber(const std::string &str) : std::string(str)
							{
							}

		int				getJSONType()
							{
								return JSONTYPE_NUMBER;
							}

		bool			isInteger()
							{
								for (std::string::iterator i = begin(); i != end(); ++i) {
									if (!isdigit(*i)) return false;
								}
								return true;
							}
};


/************************************************************************/
/*																		*/
/*	JSON Parser															*/
/*																		*/
/************************************************************************/

/*	JSException
 *
 *		In the evnet there is an error, this is thrown
 */

class JSException : public std::exception
{
	public:
		JSException(int32_t l, const char *msg)
			{
				char buffer[256];
				sprintf(buffer,"line %d: %s",l,msg);
				message = buffer;
			}

		const char *what() const throw()
			{
				return message.c_str();
			}

	private:
		std::string message;
};

/*	JSJSONParser
 *
 *		This parses an incoming request using a recursive descent parser.
 *	This returns the results as a JSONValue, with the lex parser at the
 *	appropriate position for the next object--meaning this can be called
 *	multiple times if there are multiple JSON objects in the input stream
 */

class JSJSONParser: public JSObject
{
	public:
						JSJSONParser(ref<JSInputStream> is);
						~JSJSONParser();

		ref<JSONValue>	parse();

	private:
		ref<JSLex>		lex;

		ref<JSONValue>	parseObject();
		ref<JSONValue>	parseArray();
};


#endif /* defined(__AnalyzeJSON__JSJSONParser__) */
