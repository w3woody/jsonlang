//
//  JSUtils.h
//  AnalyzeJSON
//
//  Created by William Woody on 8/17/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#ifndef AnalyzeJSON_JSUtils_h
#define AnalyzeJSON_JSUtils_h

#include <string>

extern std::string AppendIndex(const std::string &str, int index);
extern std::string FixUp(const std::string &str);
extern bool IsValidField(const std::string &str);
extern std::string EscapeString(const std::string &str);

#endif
