// -*- mode: c++; tab-width: 4; indent-tabs-mode: t -*-

/** @file
 * @author Enrico Zini <enrico@enricozini.org>
 * Quick map from package IDs to package names
 */

/*
 * Copyright (C) 2003-2007  Enrico Zini <enrico@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <ept/debtags/maint/pkgid.h>
#include <ept/debtags/maint/path.h>

namespace ept {
namespace debtags {

PkgId::PkgId() {}

PkgId::PkgId(const char* buf, int size)
	: MMap(buf, size) {}

PkgId::PkgId(const tagcoll::diskindex::MasterMMap& master, size_t idx)
	: MMap(master, idx) {}

int PkgId::byName(const std::string& name) const
{
	// Binary search the index to find the package ID
	int begin, end;

	/* Binary search */
	begin = -1, end = size();
	while (end - begin > 1)
	{
		int cur = (end + begin) / 2;
		if (byID(cur) > name)
			end = cur;
		else
			begin = cur;
	}

	if (begin == -1 || byID(begin) != name)
		//throw NotFoundException(string("looking for the ID of string ") + str);
		return -1;
	else
		return begin;
}

}
}

// vim:set ts=4 sw=4:
