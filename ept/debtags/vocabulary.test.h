/*
 * Tag vocabulary access
 *
 * Copyright (C) 2003--2007  Enrico Zini <enrico@debian.org>
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

#include <wibble/test.h>
#include <ept/debtags/vocabulary.h>
#include <ept/debtags/maint/vocabularymerger.h>
#include <ept/debtags/maint/path.h>
#include <tagcoll/utils/set.h>
#include <tagcoll/input/stdio.h>

#include "debtags.test.h"

// This is not exported by default
namespace ept {
namespace debtags {
int tagcmp(const char* tag1, const char* tag2);
}
}

using namespace std;
using namespace tagcoll::utils;
using namespace ept::debtags;

struct TestVocabulary : DebtagsTestEnvironment
{
	Vocabulary  m_tags;
	Vocabulary& tags() { return m_tags; }

        Test _1()
{
    tags(); // this will throw if the open above didn't work
}

        Test _2()
{
    assert( tags().hasFacet( "works-with" ) );
    assert( !tags().hasFacet( "blah" ) );
}

        Test _3()
{
    assert( tags().hasTag( "works-with::people" ) );
    assert( !tags().hasTag( "works-with::midgets" ) );
}

        Test _4()
{
    Tag people = tags().tagByName( "works-with::people" ),
                midgets = tags().tagByName( "works-with::midgets" ),
                blahg = tags().tagByName( "works-with::blahg" ),
                text = tags().tagByName( "works-with::text" ),
                people2 = tags().tagByName( "works-with::people" );
    assert( people != midgets );
    assert( people != text );
    assert( people != blahg );
    assert( midgets == blahg );
    assert( midgets == midgets );
    assert( people == people2 );
    assert( people == people );
}

        Test _5()
{
    Tag a = tags().tagByName( "works-with::people" ),
                b = tags().tagByName( "works-with::midgets" );
	std::set< Tag > s = tags().tags(),
                         f = tags().tags( "works-with" ),
                         n = tags().tags( "nonsense" );
    assert( set_contains(s, a) );
    assert( set_contains(f, a) );
    assert( set_contains(s, f) );
    assert( !set_contains(s, b) );
    assert( !set_contains(f, b) );
    assert( n.empty() );
}

        Test _6()
{
	Facet f = tags().facetByName( "works-with" );
    Tag t = tags().tagByName( "works-with::people" );
	assert_eq(f.name(), "works-with");
	assert_eq(t.name(), "people");
	assert_eq(t.fullname(), "works-with::people");
}

        Test _7()
{
    Facet f = tags().facetByName( "works-with" );
	std::set< Tag > x = tags().tags( "works-with" );
    assert( x == f.tags() );
}

        Test _8()
{
    Facet f = tags().facetByName( "does-not-work-with" );
    int x = 1;
    try {
        f.tags();
        x = 2;
    } catch (...) {
        x = 3;
    }
    assert_eq( x, 3 );
}

        Test _9()
{
    Facet f = tags().facetByName( "legacy" );
    assert_eq(f.shortDescription(), "");
    assert_eq(f.longDescription(), "");
    //assert_eq(f.shortDescription( "weehee" ), "weehee");
}

        Test _10()
{
	// assert that one-character tag names are parsed correctly
	assert( tags().hasTag( "implemented-in::c" ) );
}

        Test _11()
{
	// assert that all tags are somehow working
	std::set<Facet> facets = tags().facets();

	for (std::set<Facet>::const_iterator i = facets.begin();
			i != facets.end(); i++)
	{
		i->name(string("foo"));
		i->shortDescription(string("foo"));
		i->longDescription(string("foo"));
		i->tags();
	}
}

        Test _12()
{
	// assert that all tags are somehow working
	std::set<Tag> tags = this->tags().tags();

	for (std::set<Tag>::const_iterator i = tags.begin();
			i != tags.end(); i++)
	{
		i->name(string("foo"));
		i->fullname(string("foo"));
		i->shortDescription(string("foo"));
		i->longDescription(string("foo"));
	}
}

// Check for correctness of the first and last tag in the vocabulary
        Test _13()
{
	Vocabulary& tags = this->tags();

	Tag first = tags.tagByName("accessibility::TODO");
	assert(first != Tag());
	assert_eq(first.fullname(), string("accessibility::TODO"));
	assert_eq(first.name(), string("TODO"));
	assert_eq(first.shortDescription(), string("Need an extra tag"));

	Tag last = tags.tagByName("x11::xserver");
	assert(last != Tag());
	assert_eq(last.fullname(), string("x11::xserver"));
	assert_eq(last.name(), string("xserver"));
	assert_eq(last.shortDescription(), string("X Server"));
}

        Test _14()
{
	// assert that it's possible to go from facet to ID and back
	std::set<Facet> facets = tags().facets();

	for (std::set<Facet>::const_iterator i = facets.begin();
			i != facets.end(); i++)
	{
		Facet f = tags().facetByID(i->id());
		assert_eq(*i, f);
		assert_eq(i->name(), f.name());
		assert_eq(i->shortDescription(), f.shortDescription());
		assert_eq(i->longDescription(), f.longDescription());
		assert_eq(i->tags().size(), f.tags().size());
	}
}

        Test _15()
{
	// assert that it's possible to go from tag to ID and back
	std::set<Tag> tags = this->tags().tags();

	for (std::set<Tag>::const_iterator i = tags.begin();
			i != tags.end(); i++)
	{
		Tag t = this->tags().tagByID(i->id());
		assert_eq(*i, t);
		assert_eq(i->name(), t.name());
		assert_eq(i->fullname(), t.fullname());
		assert_eq(i->shortDescription(), t.shortDescription());
		assert_eq(i->longDescription(), t.longDescription());
	}
}

        Test _16()
{
	// assert that facet IDs are distinct
	std::set<Facet> facets = tags().facets();
	std::set<int> ids;
	for (std::set<Facet>::const_iterator i = facets.begin();
			i != facets.end(); i++)
		ids.insert(i->id());

	assert_eq(facets.size(), ids.size());
}

        Test _17()
{
	// assert that tag IDs are distinct
	std::set<Tag> tags = this->tags().tags();
	std::set<int> ids;
	for (std::set<Tag>::const_iterator i = tags.begin();
			i != tags.end(); i++)
		ids.insert(i->id());

	assert_eq(tags.size(), ids.size());
}

        Test _18()
{
	// assert that all the tags are indexed
	ept::debtags::VocabularyMerger voc;
	tagcoll::input::Stdio in(ept::debtags::Path::vocabulary());
	voc.read(in);
	std::set<std::string> all = voc.tagNames();
	for (std::set<std::string>::const_iterator i = all.begin();
			i != all.end(); ++i)
		assert(this->tags().hasTag(*i));

	// There should be the same amount of tags in both
	std::set<Tag> allTags = this->tags().tags();
	assert_eq(all.size(), allTags.size());
}

        Test _19()
{
	// test the tagcmp function

	// If unfaceted, same as strcmp
	assert(ept::debtags::tagcmp("antani", "blinda") < 0);
	assert(ept::debtags::tagcmp("blinda", "antani") > 0);
	assert_eq(ept::debtags::tagcmp("antani", "antani"), 0);

	// If the same and faceted, should work
	assert_eq(ept::debtags::tagcmp("antani::blinda", "antani::blinda"), 0);

	// With different facet names, work just as strcmp
	assert(ept::debtags::tagcmp("antani::blinda", "blinda::blinda") < 0);
	assert(ept::debtags::tagcmp("blinda::blinda", "antani::blinda") > 0);
	assert(ept::debtags::tagcmp("anta::blinda", "antani::blinda") < 0);
	assert(ept::debtags::tagcmp("antani::blinda", "anta::blinda") > 0);
	assert(ept::debtags::tagcmp("anta::blinda", "anta-ni::blinda") < 0);
	assert(ept::debtags::tagcmp("anta-ni::blinda", "anta::blinda") > 0);

	// With same facet names, work just as strcmp on the tags
	assert(ept::debtags::tagcmp("a::antani", "a::blinda") < 0);
	assert(ept::debtags::tagcmp("a::blinda", "a::antani") > 0);
	assert(ept::debtags::tagcmp("a::anta", "a::antani") < 0);
	assert(ept::debtags::tagcmp("a::antani", "a::anta") > 0);
	assert(ept::debtags::tagcmp("a::anta", "a::anta-ni") < 0);
	assert(ept::debtags::tagcmp("a::anta-ni", "a::anta") > 0);
}

        Test _20()
{
	// check that we're seeing all the tags for a facet
	std::set<Tag> t = tags().tags("accessibility");
	assert_eq(t.size(), 10u);

	t = tags().tags("works-with-format");
	assert_eq(t.size(), 33u);
}

// If there is no data, Vocabulary should work as an empty vocabulary
        Test _21()
{
	Path::OverrideDebtagsSourceDir odsd("./empty");
	Path::OverrideDebtagsIndexDir odid("./empty");
	Path::OverrideDebtagsUserSourceDir odusd("./empty");
	Path::OverrideDebtagsUserIndexDir oduid("./empty");
	Vocabulary empty;

	assert(!empty.hasData());

	set<Facet> facets = empty.facets();
	assert_eq(facets.size(), 0u);

	set<Tag> tags = empty.tags();
	assert_eq(tags.size(), 0u);
}

};

// vim:set ts=4 sw=4:
