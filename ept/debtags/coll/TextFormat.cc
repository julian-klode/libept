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

#include "TextFormat.h"
#include "fast.h"
#include "operators.h"
#include <stdexcept>
#include <system_error>
#include <set>

using namespace std;
using namespace ept::debtags::coll::operators;

namespace ept {
namespace debtags {
namespace coll {
namespace textformat {

// Parse an element
// Return the trailing separating char, that can be:
//  input::Input::Eof
//  '\n'
//  ':'
//  ','
// Return the item in `item'

// element: \s*[^ \t,:]\s*([.:])\s*
// or
// element: \s*[^ \t,:].*?[^ \t,:]\s*([.:])\s+
int parseElement(FILE* in, const std::string& pathname, string& item)
{
    item = string();
    string sep;
    int c;
    char sepchar = 0;
    enum {LSPACE, ITEM, ISPACE, ISEP, TSPACE} state = LSPACE;
    while ((c = getc(in)) != EOF)
    {
        if (c == '\n')
        {
            if (sepchar && sepchar != ':')
                throw std::runtime_error("separator character ends the line");
            else
                return '\n';
        }
        switch (state)
        {
            // Optional leading space
            case LSPACE:
                switch (c)
                {
                    case ' ':
                    case '\t':
                        break;
                    case ':':
                    case ',':
                        throw std::runtime_error("element cannot start with a separation character");
                        break;
                    default:
                        item += c;
                        state = ITEM;
                        break;
                }
                break;
            // Non-separating characters
            case ITEM:
                switch (c)
                {
                    case ' ':
                    case '\t':
                        sep += c;
                        state = ISPACE;
                        break;
                    case ':':
                    case ',':
                        sepchar = c;
                        sep += c;
                        state = ISEP;
                        break;
                    default:
                        item += c;
                        break;
                }
                break;
            // Space inside item or at the end of item
            case ISPACE:
                switch (c)
                {
                    case ' ':
                    case '\t':
                        sep += c;
                        break;
                    case ':':
                    case ',':
                        sepchar = c;
                        state = TSPACE;
                        break;
                    default:
                        item += sep;
                        item += c;
                        sep = string();
                        state = ITEM;
                        break;
                }
                break;
            // Separator inside item or at the end of item
            case ISEP:
                switch (c)
                {
                    case ' ':
                    case '\t':
                        if (sep.size() > 1)
                            throw std::runtime_error("item is followed by more than one separator characters");
                        state = TSPACE;
                        break;
                    case ':':
                    case ',':
                        sep += c;
                        break;
                    default:
                        item += sep;
                        item += c;
                        sepchar = 0;
                        sep = string();
                        state = ITEM;
                        break;
                }
                break;
            case TSPACE:
                switch (c)
                {
                    case ' ':
                    case '\t':
                        break;
                    default:
                        ungetc(c, in);
                        return sepchar;
                }
                break;
        }
    }
    if (ferror(in))
        throw std::system_error(errno, std::system_category(), "cannot read from " + pathname);
    return EOF;
}


// item1, item2, item3: tag1, tag2, tag3

//#define TRACE_PARSE
void parse(FILE* in, const std::string& pathname, Fast& out)
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
                        out.insert(itemset, std::set<std::string>());
                    else
                        out.insert(itemset, tagset);
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
}
}
