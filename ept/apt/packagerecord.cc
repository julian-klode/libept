/** \file
 * Parser for APT records, with specialised accessors for package records
 */

/* 
 * Copyright (C) 2007  Enrico Zini <enrico@enricozini.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include <ept/apt/packagerecord.h>

#include <cctype>
#include <cstdlib>

//#include <iostream>

using namespace std;

namespace ept {
namespace apt {

size_t PackageRecord::parseSize(size_t def, const std::string& str) const
{
	if (str == string())
		return def;
	return (size_t)strtoul(str.c_str(), NULL, 10);
}

std::string PackageRecord::parseShortDescription(const std::string& def, const std::string& str) const
{
	if (str == std::string())
		return def;
	size_t pos = str.find("\n");
	if (pos == std::string::npos)
		return str;
	else
		return str.substr(0, pos);
}

std::string PackageRecord::parseLongDescription(const std::string& def, const std::string& str) const
{
	if (str == std::string())
		return def;
	size_t pos = str.find("\n");
	if (pos == std::string::npos)
		return str;
	else
	{
		// Trim trailing spaces
		for (++pos; pos < str.size() && isspace(str[pos]); ++pos)
			;
		return str.substr(pos);
	}
}

namespace {

struct Tagparser
{
    set<string>& res;
    string cur_tag;
    bool has_braces;

    Tagparser(set<string>& res) : res(res) {}

    void expand_tag_braces(const std::string& tag)
    {
        size_t begin = tag.find('{');
        if (begin != string::npos)
        {
            string prefix(tag, 0, begin);
            ++begin;
            size_t end;
            while ((end = tag.find(',', begin)) != string::npos)
            {
                res.insert(prefix + tag.substr(begin, end-begin));
                begin = end + 1;
            }
            res.insert(prefix + tag.substr(begin, tag.size() - 1 - begin));
        }
    }

    void reset_tag()
    {
        cur_tag.clear();
        has_braces = false;
    }

    void have_tag()
    {
        if (has_braces)
            expand_tag_braces(cur_tag);
        else
            res.insert(cur_tag);
        reset_tag();
    }

    void parse(const std::string& s)
    {
        enum State {
            SEP,
            TAG,
            BRACES,
        } state = SEP;

        reset_tag();

        // Tokenize, dealing with braces
        for (string::const_iterator c = s.begin(); c != s.end();)
        {
            switch (state)
            {
                case SEP:
                    switch (*c)
                    {
                        case ' ':
                        case '\t':
                        case '\n':
                        case ',':
                            ++c;
                            break;
                        default:
                            state = TAG;
                            break;
                    }
                    break;
                case TAG:
                    switch (*c)
                    {
                        case ' ':
                        case '\t':
                        case '\n':
                        case ',':
                            ++c;
                            have_tag();
                            state = SEP;
                            break;
                        case '{':
                            cur_tag += *c;
                            ++c;
                            has_braces = true;
                            state = BRACES;
                            break;
                        default:
                            cur_tag += *c;
                            ++c;
                            break;
                    }
                    break;
                case BRACES:
                    cur_tag += *c;
                    ++c;
                    switch (*c)
                    {
                        case '}':
                            state = TAG;
                            break;
                    }
                    break;
            }
        }
        if (!cur_tag.empty())
            have_tag();
    }
};

}

std::set<std::string> PackageRecord::parseTags(const std::set<std::string>& def, const std::string& str) const
{
    if (str == string())
        return def;

    set<string> res;

    Tagparser parser(res);
    parser.parse(str);

    return res;
}

}
}

// vim:set ts=4 sw=4:
