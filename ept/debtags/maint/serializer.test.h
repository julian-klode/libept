// -*- mode: c++; tab-width: 4; indent-tabs-mode: t -*-
/**
 * @file
 * @author Enrico Zini (enrico) <enrico@enricozini.org>
 */

/*
 * Tests for Debtags serialization filters
 *
 * Copyright (C) 2003-2007  Enrico Zini <enrico@debian.org>
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

#include <ept/debtags/maint/serializer.h>
#include <ept/debtags/maint/pkgid.h>
#include <ept/debtags/maint/path.h>
#include <ept/debtags/vocabulary.h>
#include <ept/debtags/debtags.h>

#include <tagcoll/coll/simple.h>

#include <wibble/singleton.h>

#include <ept/test.h>

using namespace std;
using namespace tagcoll;
using namespace ept;
using namespace ept::debtags;

struct TestSerializer : DebtagsTestEnvironment
{
	Debtags debtags;
	Vocabulary& voc;
	PkgId& pkgid;

	TestSerializer()
		: voc(debtags.vocabulary()), pkgid(debtags.pkgid()) {}

/* Test going from a stream of tag data <string, string> to a stream of tag
 * data <int, int> to a stream of tag data <Package, Tag> and finally back to a
 * stream of tag data <string, string>
 */
	Test _1()
{
	// Source data <string, string>
	coll::Simple<string, string> source;
	source.insert(wibble::singleton(string("debtags")), wibble::singleton(string("use::editing")));
	source.insert(wibble::singleton(string("debtags")), wibble::singleton(string("role::program")));

	// <string, string> -> <int, int>
	coll::Simple<int, int> dest;
	source.output(stringToInt(pkgid, voc, inserter(dest)));

	assert_eq(dest.itemCount(), 1u);
	assert_eq(dest.tagCount(), 2u);

	// <int, int> -> <Package, Tag>
	coll::Simple<string, Tag> dest1;
	dest.output(intToPkg(pkgid, voc, inserter(dest1)));

	assert_eq(dest1.itemCount(), 1u);
	assert_eq(dest1.tagCount(), 2u);

	std::set<Tag> tags = dest1.getTagsOfItem("debtags");
	assert_eq(tags.size(), 2u);

	Tag useEditing = voc.tagByName("use::editing");
	Tag roleProgram = voc.tagByName("role::program");

	assert(tags.find(useEditing) != tags.end());
	assert(tags.find(roleProgram) != tags.end());

	// <Package, Tag> -> <string, string>
	coll::Simple<string, string> dest2;
	dest1.output(pkgToString(inserter(dest2)));

	assert_eq(dest2.itemCount(), 1u);
	assert_eq(dest2.tagCount(), 2u);

	std::set<std::string> tags1 = dest2.getTagsOfItem("debtags");
	assert_eq(tags1.size(), 2u);

	assert(tags1.find("use::editing") != tags1.end());
	assert(tags1.find("role::program") != tags1.end());
}

/* Test going from patch with strings to patch with ints and vice versa */
	Test _2()
{
	PatchList<string, string> change;
	change.addPatch(Patch<string, string>("debtags",
				wibble::singleton(string("use::gameplaying")),
				wibble::singleton(string("use::editing"))));

	// Deserialise to ints
	PatchList<int, int> intChange;
	change.output(patchStringToInt(pkgid, voc, tagcoll::inserter(intChange)));
	assert_eq(intChange.size(), 1u);
	assert_eq(intChange.begin()->second.added.size(), 1u);
	assert_eq(intChange.begin()->second.removed.size(), 1u);
	
	// Serialise back to strings
	PatchList<string, string> change1;
	intChange.output(patchIntToString(pkgid, voc, tagcoll::inserter(change1)));
	assert_eq(change1.size(), 1u);
	assert_eq(change1.begin()->first, string("debtags"));
	assert_eq(change1.begin()->second.item, string("debtags"));
	assert_eq(change1.begin()->second.added.size(), 1u);
	assert_eq(*change1.begin()->second.added.begin(), string("use::gameplaying"));
	assert_eq(change1.begin()->second.removed.size(), 1u);
	assert_eq(*change1.begin()->second.removed.begin(), string("use::editing"));
}

};

#include <tagcoll/coll/simple.tcc>
#include <tagcoll/patch.tcc>

// vim:set ts=4 sw=4:
