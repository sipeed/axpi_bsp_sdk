/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#ifndef AXSTRINGUTILS_H
#define AXSTRINGUTILS_H

using namespace std;

class CStringUtils
{
public:
    CStringUtils(void);
    ~CStringUtils(void);

    template<typename ... Args>
    static string string_format(const string& format, Args ... args)
    {
        size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args ...);  // Extra space for \0
        char bytes[size];
        snprintf(bytes, size, format.c_str(), args ...);
        return string(bytes);
    }

    static bool string_split(vector<string>& tokens, string text, const string delimiters=",")
    {
        string::size_type lastPos = text.find_first_not_of(delimiters, 0);
        string::size_type pos = text.find_first_of(delimiters, lastPos);
        while (string::npos != pos || string::npos != lastPos) {
            tokens.emplace_back(text.substr(lastPos, pos - lastPos));
            lastPos = text.find_first_not_of(delimiters, pos);
            pos = text.find_first_of(delimiters, lastPos);
        }
        
        return true;
    }
};

#endif // AXSTRINGUTILS_H