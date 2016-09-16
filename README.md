# JL: Generate code to parse JSON on iOS and Android.

One of the problems with developing iOS or Android software is integrating against a back-end service. It's not "hard", but the nit-picky details can be a pain.

The purpose of the JL suite of tools is to provide a standard encoding representation for representing an arbitrary data structure sent via the JSON protocol, and to provide tools for reverse engineering JSON requests into the encoding representation.

## Compile the tools

The tools described here can be built using the latest version of Xcode.

# The JSON abstract syntax language.

The JSON abstract syntax language describes the structure of objects as they are sent via JSON. We assume that objects represent an underlying structural representation that is somewhat orderly: arrays are homogeneous, and similarly named fields contain similar contents. While this is not necessarily true in JSON: array contents do not need to be homogenous, the JL language accommodates this through the use of a generic 'object' declaration and "optional" modifier keywords.

Each JSON abstract syntax language file ends with the suggested extension '.jl', and consists of a series of objects. Each object contains one or more fields which can be either a primitive type, an array of objects, or an object itself. Thus:

    /* An example .jl file showing the declaration of three objects */
            
    /*  Feed 
     *
     *      Top level of the feed
     */

    Feed {
        id: integer,
        name: string,
        date: string,
        active: boolean,
        addressList: arrayof Address,
        phoneList: arrayof Phone,
    }

    /*  Address
     *
     *      The user's address
     */

    Address {
        id: integer,
        name: string,
        address: string,
        address2: (optional) string, // optional in the data stream
        city: string,
        state: string,
        zip: string
    }

    /*  Phone
     *
     *      The user's phone
     */

    Phone {
        id: integer,
        name: string,
        phone: string
    }

The above defines three types of objects, including one which contains two arrays. The above, if we were to deserialize starting with the 'Feed' object, would successfully deserialize the following JSON:

    {
        "id": 5,
        "name": "John Doe",
        "active": true,
        "date": "1965/10/03",
        addressList: [
            {
                "id": 111,
                "name": "Home",
                "address": "123 Main St",
                "city": "Anytown",
                "state": "XX",
                "zip": "12345"
            }
        ],
        phoneList: []
    }

The basic language simply defines the objects that are to be parsed, on the assumption that the JSON being returned contains a complex data structure. Arrays are always assumed to be part of an object, and if the return from a JSON object is an array, it is assumed that your code will iterate the contents of the JSON array and convert the content objects one at a time.

A sorta formal description of the JSON abstract syntax language.

Comments The .jl compiler described below supports C style comments and C++ style comments. That is, /* ... */ comments and // ... \n comments are both supported.

The language is described below.

    -- This is an informal description of the JSON abstract language syntax.
        
    -- The top level language is simply a list of object declarations.
    jl :== symbol_decl_list ;

    object_decl_list :== object_decl
                     |   object_decl object_decl_list
                     ;

    -- An individual object is a token, followed by curly braces and a list of the members
                 
    object_decl :== TOKEN '{' member_list '}'
                |   TOKEN : parent_decl_list '{' member_list '}'  -- new inheritance
                ;

    parent_decl_list :== TOKEN
                     |   TOKEN ',' parent_decl_list
                     ;

    -- Members are comma separated

    member_list :== member_decl
                |   member_decl ',' member_list
                ;

    -- A member is the name, and then the declaration type. The name can be either a token
    -- or an arbitrary string.

    member_list :== TOKEN ':' decl
                |   STRING ':' decl

    -- A declaration can have modifiers.

    decl :== modifier_list primtype
         |   primtype
         ;

    -- Modifiers are surrounded by parenthesis, and can be either 'optional', indicating that
    -- the field is optional, or 'nullable' indicating that 'null' is a valid value for this
    -- item. 'nullable' can be used for array items and object fields; if 'nullable' is not
    -- provided, but the field is set to null, the results are undefined.

    modifier_list :== '(' modifiers ')'

    modifiers :== modifier modifiers
              |   modifier
              ;

    modifier :== 'nullable'
             |   'optional'
             ;

    -- Primitive declaration types include a symbol declaration (declared as an object above),
    -- or can be an array or a JSON primitive type. The type 'object' means a generic object,
    -- meaning it can be any valid JSON value.

    -- Also note we distinguish between an integer type and a real type. JSON encodes all
    -- values as real; however, the 'integer' declaration forces parsing and conversion into
    -- an integer type

    primtype :== symbol
             |   array_decl
             |   'string',
             |   'integer',
             |   'real',
             |   'boolean'   -- true / false
             |   'object'
             ;

    -- Array types can be nested.

    array_decl | 'arrayof' decl ;

    -- Symbol must be declared somewhere in the .jl file as an object.

    symbol : TOKEN ;

## Command Line Tools

There are two tools which support this command suite. The first can take a JSON file and make a best guess at a representational language which can parse this file. The representational language is somewhat conservative, but with some human intervention could be optimized to properly represent the server response.

### AnalyzeJSON

    Usage:
        
    AnalyzeJSON (infile) (outfile)

    This will take the input file, presumed to be one or more complete JSON 
    objects, and generates a template .jl file which is a best guess at the 
    structure of the input file.

    AnalyzeJSON

    Without any command line arguments, reads JSON from the standard input
    until an EOF is reached, then generates the .jl file to standard out.

    In either case, a syntax error will generate an error message to
    standard out.

The second tool takes a .jl file and generates Objective C and/or Java which can be used to parse JSON data. On Objective C, the JSON data is assumed to have been parsed into memory first into NSDictionary/NSArray objects. In Java, this assumes (### what? TODO)

### CompileJL

    CompileJL [-objc (class prefix)] [-java (class path)] [-readonly] (infile) (outdir)

    Parses a .jl file from infile and generates the class files needed to parse the
    specification language into the output directory specified. If the directory does
    not exist, one is created.

    -objc (class prefix)

    Specifies that Objective C is to be generated. Also specifies that the prefix string
    is to be attached to the object types declared in the .jl file. So, for example, if
    this was specified as -objc CMJson, then the following files
    would be generated:

        CMJsonFeed.m
        CMJsonFeed.h
        CMJsonAddress.m
        CMJsonAddress.h
        CMJsonPhone.m
        CMJsonPhone.h
    
    -java (class path)

    Specifies that Java is to be generated. When used on the above example, if this is given
    as -java com.test, then this generates the class files

        Feed.java
        Address.java
        Phone.java

    inside the directory specified, with classpath "com.test".

    -readonly

    Specifies that the classes will be read-only. This will omit generating the code
    for writing these classes to a JSON stream.

## Inheritance

The latest version that was uploaded adds "inheritance" to the object declaration mechanism. This allows you to define a common underlying JSON object, and use that as a template for other JSON objects. For example, if your server commonly returns a status field and an optional error message with each return, you can declare a common error class:

    ErrorCode {
        success: boolean,
        errorCode: (optional) integer,
        errorMessage: (optional) string
    }

Then you can declare your response objects, inheriting from the error code object previously declared:

    ReturnMessage : ErrorCode {
        message: string
    }

This would be the equivalent of declaring:

    ReturnMessage {
        success: boolean,
        errorCode: (optional) integer,
        errorMessage: (optional) string,
        message: string
    }

Note that the current code generators do not declare your JSON objects with inheritance. Also note that it is illegal to have a duplicate field declaration, even if that happens through inheritance.

# License
    JL: A simple application to help build JSON parsers
    
    Copyright © 2014, 2016 by William Edward Woody
    
    This program is free software: you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation, either version 3 of the License, or 
    (at your option) any later version.

    This program is distributed in the hope that it will be useful, but 
    WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
    General Public License for more details.
    
    You should have received a copy of the GNU General Public License 
    along with this program. If not, see http://www.gnu.org/licenses/
    
    Contact information:
    
    William Edward Woody
    12605 Raven Ridge Rd
    Raleigh, NC 27614
    United States of America
    woody@alumni.caltech.edu


William Woody  
woody@alumni.caltech.edu
