//
//  JSObject.cpp
//  AnalyzeJSON
//
//  Created by William Woody on 8/15/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#include <stdio.h>
#include "JSObject.h"

/************************************************************************/
/*																		*/
/*	Basic Object Definitions											*/
/*																		*/
/************************************************************************/

JSObject::JSObject()
{
	refCount = 0;
}

JSObject::~JSObject()
{
	if (refCount != 0) {
		fprintf(stderr,"refcount not zero");
	}
}

