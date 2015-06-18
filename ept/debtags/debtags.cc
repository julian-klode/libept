/** -*- C++ -*-
 * @file
 * @author Enrico Zini (enrico) <enrico@enricozini.org>
 */

/*
 * System tag database
 *
 * Copyright (C) 2003-2008  Enrico Zini <enrico@debian.org>
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

#include <ept/debtags/debtags.h>

#include <tagcoll/patch.h>
#include <tagcoll/coll/simple.h>
#include <tagcoll/input/stdio.h>
#include <tagcoll/TextFormat.h>

#include <wibble/sys/fs.h>
#include <wibble/string.h>

#include <iostream>
#include <sstream>

#include <sys/wait.h>	// WIFEXITED WEXITSTATUS
#include <sys/types.h>	// getpwuid, getuid
#include <pwd.h>	// getpwuid
#include <unistd.h>	// getuid


using namespace std;
using namespace tagcoll;
using namespace wibble;

namespace ept {
namespace debtags {

Debtags::Debtags(bool editable)
    : m_timestamp(0)
{
    string src = pathname();
    if (!sys::fs::exists(src))
        return;

    // Read uncompressed data
    tagcoll::input::Stdio in(src);

    // Read the collection
    tagcoll::textformat::parse(in, inserter(*this));

    // Read the timestamp
    m_timestamp = sys::fs::timestamp(src, 0);
}

string Debtags::pathname()
{
    const char* res = getenv("DEBTAGS_TAGS");
    if (!res) res = "/var/lib/debtags/package-tags";
    return res;
}

}
}

#include <tagcoll/coll/simple.tcc>
#include <tagcoll/coll/fast.tcc>
#include <tagcoll/TextFormat.tcc>

// Explicit template instantiations for our stuff
template class tagcoll::coll::Fast<std::string, std::string>;

// vim:set ts=4 sw=4:
