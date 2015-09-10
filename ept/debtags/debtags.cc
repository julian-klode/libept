/** -*- C++ -*-
 * @file
 * @author Enrico Zini (enrico) <enrico@enricozini.org>
 */

/*
 * System tag database
 *
 * Copyright (C) 2003-2015  Enrico Zini <enrico@debian.org>
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

#include "debtags.h"
#include "ept/utils/sys.h"
#include "coll/TextFormat.h"
#include <system_error>
#include <iostream>
#include <sstream>
#include <sys/wait.h>	// WIFEXITED WEXITSTATUS
#include <sys/types.h>	// getpwuid, getuid
#include <pwd.h>	// getpwuid
#include <unistd.h>	// getuid

using namespace std;

namespace ept {
namespace debtags {

Debtags::Debtags()
    : m_timestamp(0)
{
    string src = pathname();
    if (!sys::exists(src))
        return;
    load(src);
}

Debtags::Debtags(const std::string& pathname)
    : m_timestamp(0)
{
    load(pathname);
}

void Debtags::load(const std::string& pathname)
{
    // Read uncompressed data
    FILE* in = fopen(pathname.c_str(), "rt");
    if (!in)
        throw std::system_error(errno, std::system_category(), "cannot open " + pathname);

    // Read the collection
    try {
        coll::textformat::parse(in, pathname, *this);
    } catch (...) {
        fclose(in);
        throw;
    }
    fclose(in);

    // Read the timestamp
    m_timestamp = sys::timestamp(pathname, 0);
}

string Debtags::pathname()
{
    const char* res = getenv("DEBTAGS_TAGS");
    if (!res) res = "/var/lib/debtags/package-tags";
    return res;
}

}
}
