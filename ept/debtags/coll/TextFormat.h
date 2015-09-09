#ifndef TAGCOLL_TEXTFORMAT_H
#define TAGCOLL_TEXTFORMAT_H

/** \file
 * Serialize and deserialize a tagged collection to a text file
 */

/*
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

#include <wibble/mixin.h>
#include <wibble/empty.h>
#include <wibble/singleton.h>
//#include <tagcoll/input/base.h>
#include <cstdio>

//#define TRACE_PARSE

namespace tagcoll
{

namespace textformat
{

/**
 * TagcollConsumer that serializes its input to an output stream
 *
 * The format of the output is:
 *   lines of "comma+space"-separated items, followed by "colon+space",
 *   followed by the corresponding "comma+space"-separated tags.
 * Examples:
 *   ITEM:
 *   ITEM: TAG
 *   ITEM: TAG1, TAG2, TAG3
 *   ITEM1, ITEM2, ITEM3:
 *   ITEM1, ITEM2, ITEM3: TAG1, TAG2, TAG3
 */
class StdioWriter : public wibble::mixin::OutputIterator<StdioWriter>
{
protected:
    FILE* out;

public:
    StdioWriter(FILE* out) : out(out) {}

    template<typename Items, typename Tags>
    StdioWriter& operator=(const std::pair<Items, Tags>& data);
};

class OstreamWriter : public wibble::mixin::OutputIterator<OstreamWriter>
{
protected:
    std::ostream& out;

public:
    OstreamWriter(std::ostream& out) : out(out) {}

    template<typename Items, typename Tags>
    OstreamWriter& operator=(const std::pair<Items, Tags>& data);
};

/**
 * Parse an element from input
 *
 * @retval item
 *   The item found on input
 * @return
 *   the trailing separating char, that can be:
 *   \li input::Input::Eof
 *   \li '\n'
 *   \li ':'
 *   \li ','
 */
int parseElement(FILE* in, const std::string& pathname, std::string& item);


/*
 * Parse a tagged collection, sending the results to out.
 *
 * @param out
 *   An output iterator accepting a std::pair<string, string>
 */
template<typename OUT>
void parse(FILE* in, const std::string& pathname, OUT out);

}
}

// vim:set ts=4 sw=4:
#endif
