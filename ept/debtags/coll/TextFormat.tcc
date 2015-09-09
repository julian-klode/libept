/*
 * Serialize a tagged collection to a text file
 *
 * Copyright (C) 2003--2015  Enrico Zini <enrico@debian.org>
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

#ifndef TAGCOLL_TEXTFORMAT_TCC
#define TAGCOLL_TEXTFORMAT_TCC

#include <ept/debtags/coll/TextFormat.h>
//#include <tagcoll/patch.h>

#include <wibble/exception.h>
#include <wibble/empty.h>
#include <wibble/operators.h>

#include <ostream>

using namespace std;
using namespace wibble;
using namespace wibble::operators;

static void printTagset(const std::set<string>& ts, FILE* out)
{
    for (std::set<string>::const_iterator i = ts.begin();
            i != ts.end(); i++)
        if (i == ts.begin())
        {
            if (fprintf(out, "%s", i->c_str()) < 0)
                throw wibble::exception::System("writing tagset");
        }
        else
        {
            if (fprintf(out, ", %s", i->c_str()) < 0)
                throw wibble::exception::System("writing tagset");
        }
}

namespace tagcoll {
namespace textformat {

inline static void outString(const std::string& str, FILE* out, const char* what)
{
    if (fwrite(str.data(), str.size(), 1, out) != 1)
        throw wibble::exception::System(string("writing ") + what);
}

template<typename Items, typename Tags>
StdioWriter& StdioWriter::operator=(const std::pair<Items, Tags>& data)
{
    for (typename Items::const_iterator i = data.first.begin();
            i != data.first.end(); ++i)
    {
        if (i != data.first.begin())
            if (fputs(", ", out) == EOF)
                throw wibble::exception::System("writing comma after item");
        outString(*i, out, "item");
    }
    if (data.second.begin() != data.second.end())
    {
        if (fputs(": ", out) == EOF)
            throw wibble::exception::System("writing colon after items");
        for (typename Tags::const_iterator i = data.second.begin();
                i != data.second.end(); ++i)
        {
            if (i != data.second.begin())
                if (fputs(", ", out) == EOF)
                    throw wibble::exception::System("writing comma after tag");
            outString(*i, out, "tag");
        }
    }
    if (fputc('\n', out) == EOF)
        throw wibble::exception::System("writing newline after tagset");
    return *this;
}

template<typename Items, typename Tags>
OstreamWriter& OstreamWriter::operator=(const std::pair<Items, Tags>& data)
{
    for (typename Items::const_iterator i = data.first.begin();
            i != data.first.end(); ++i)
    {
        if (i != data.first.begin())
            out << ", ";
        out << *i;
    }
    if (data.second.begin() != data.second.end())
    {
        out << ": ";
        for (typename Tags::const_iterator i = data.second.begin();
                i != data.second.end(); ++i)
        {
            if (i != data.second.begin())
                out << ", ";
            out << *i;
        }
    }
    out << endl;
    return *this;
}



// item1, item2, item3: tag1, tag2, tag3

//#define TRACE_PARSE
template<typename OUT>
void parse(FILE* in, const std::string& pathname, OUT out)
{
    string item;

    std::set<string> itemset;
    std::set<string> tagset;
    int sep;
    enum {ITEMS, TAGS} state = ITEMS;
    int line = 1;
    do
    {
        sep = parseElement(in, pathname, item);
        
        if (item.size() != 0)
        {
            if (state == ITEMS)
                itemset |= item;
            else
                tagset |= item;
        }
        
        switch (sep)
        {
            case '\n':
                line++;
            case EOF:
                if (!(itemset.empty() && tagset.empty()))
                {
                    if (itemset.empty())
                        throw std::runtime_error("no elements before ':' separator");
                    if (tagset.empty())
                        *out = make_pair(itemset, wibble::Empty<std::string>());
                    else
                        *out = make_pair(itemset, tagset);
                    ++out;
                }
                itemset.clear();
                tagset.clear();
                state = ITEMS;
                break;
            case ':':
                if (state == TAGS)
                    throw std::runtime_error("separator ':' appears twice");
                state = TAGS;
                break;
            default:
                break;
        }
    } while (sep != EOF);
}

}
}

#endif
