/*
 * sformat.cpp
 *
 *  Created on: Feb 26, 2016
 *      Author: peter
 */

#include "sformat.h"

namespace sformat
{

    Format parseFormat(const char *& p)
    {
        Format result;
        bool last_static = false;
        while (*p && *p != '}')
        {
            std::string sta;
            while (*p)
            {
                if (*p == '~' && p[1])
                    p++;
                else if (*p == '{' || *p == '}')
                    break;
                sta += *p++;
            }
            if (sta.size())
            {
                if (!last_static)
                {
                    result.push_back(Field("", ""));
                    last_static = true;
                }
                result.back().options += sta;
            }
            if (*p == '{')
            {
                p++;
                std::string name;
                while (*p)
                {
                    if (*p == '~' && p[1])
                        p++;
                    else if (*p == ':' || *p == '}')
                        break;
                    name += *p++;
                }
                std::string options;
                std::vector<Format> subformats;
                if (*p == ':')
                {
                    p++;
                    while (*p)
                    {
                        if (*p == '~' && p[1])
                            p++;
                        else if (*p == ':' || *p == '}')
                            break;
                        options += *p++;
                    }
                    while (*p == ':')
                    {
                        p++;
                        subformats.push_back(parseFormat(p));
                    }
                }
                if (*p != '}')
                    throw std::runtime_error("'}' expected");
                result.push_back(Field(name, options));
                result.back().subformats.swap(subformats);
                last_static = false;
                p++;
            }
        }
        return result;
    }

    Format fmt(const std::string& s)
    {
        const char *p = s.c_str();
        return parseFormat(p);
    }

    Format fmt(const char *p)
    {
        return parseFormat(p);
    }

}
