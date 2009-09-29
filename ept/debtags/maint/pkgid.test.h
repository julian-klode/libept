// -*- mode: c++; tab-width: 4; indent-tabs-mode: t -*-
/*
 * id->package mapping
 *
 * Copyright (C) 2006  Enrico Zini <enrico@debian.org>
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
#include <ept/debtags/debtags.h>
#include <set>

#include <ept/test.h>

using namespace std;
using namespace ept;
using namespace ept::debtags;

struct TestPkgid : DebtagsTestEnvironment
{
	Debtags debtags;
	PkgId& pkgid;

	TestPkgid()
		: pkgid(debtags.pkgid())
	{
	}

// Check that we can go from name to ID and back
	Test _1()
{
	//int x = 0;
	for (Debtags::const_iterator i = debtags.begin();
			i != debtags.end(); ++i)
	{
		int id = pkgid.byName(i->first);
		std::string pkg = pkgid.byID(id);
		assert(i->first == pkg);

		/* std::cerr << x << ": " << i->id() << ": "
				  << i->name() << ", " << pkgidx().name( i->id() ) <<
				  std::endl; */
		//++ x;
	}
}

// Check that IDs are distinct
	Test _2()
{
	using namespace std;

	size_t count = 0;
	set<int> ids;
	for (Debtags::const_iterator i = debtags.begin(); i != debtags.end(); ++i, ++count)
		ids.insert(pkgid.byName(i->first));
	assert_eq(ids.size(), count);
}

};

// vim:set ts=4 sw=4:
