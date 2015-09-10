#ifndef EPT_DEBTAGS_COLL_TEXTFORMAT_H
#define EPT_DEBTAGS_COLL_TEXTFORMAT_H

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

#include <cstdio>
#include <string>

//#define TRACE_PARSE

namespace ept {
namespace debtags {
namespace coll {
struct Fast;

namespace textformat {

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
void parse(FILE* in, const std::string& pathname, Fast& out);

}
}
}
}

#endif
