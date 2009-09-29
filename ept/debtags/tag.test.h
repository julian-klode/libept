/*
 * Copyright (C) 2005,2007  Enrico Zini <enrico@debian.org>
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

#if 0
#include <ept/tests/test-utils.h>
#include <ept/debtags/tag.h>
#include <ept/debtags/vocabulary.h>
#include <ept/debtags/maint/path.h>

#include <tagcoll/utils/set.h>

using namespace std;
using namespace ept::debtags;

namespace tut {

struct ept_debtags_tag_shar
{
	Path::OverrideDebtagsSourceDir odsd;
	Path::OverrideDebtagsIndexDir odid;
	Path::OverrideDebtagsUserSourceDir odusd;
	Path::OverrideDebtagsUserIndexDir oduid;
	Vocabulary voc;

	ept_debtags_tag_shar()
		: odsd("./"), odid("./"), odusd("./"), oduid("./") {}
};

TESTGRP( ept_debtags_tag );

template<> template<>
void to::test<1>()
{
	Tag a, b;
	ensure( a == b );
	ensure( !a.valid() );
	ensure( !b.valid() );
}

template<> template<>
void to::test<2>()
{
	Tag a;
	int x = 1;
	try {
		a.shortDescription();
		x = 2;
	} catch (...) {
		x = 3;
	}
	ensure_equals( x, 3 );
}

template<> template<>
void to::test< 3 >()
{
	Facet f = voc.facetByName( "works-with" );
	Tag t = voc.tagByName( "works-with::people" );
	ensure( t.valid() );
	ensure( f.valid() );
	ensure( t.facet() == f );
	ensure( tagcoll::utils::set_contains(f.tags(), t) );
}

template<> template<>
void to::test< 4 >()
{
	Facet f = voc.facetByName( "works-with" );
	Tag t = voc.tagByName( "works-with::people" );
	ensure( t.valid() );
	ensure( f.valid() );
	ensure( f.hasTag( t.name() ) );
}

template<> template<>
void to::test< 5 >()
{
	Tag t = voc.tagByName( "works-with::people" );
	ensure( t.valid() );
	ensure( t.facet().hasTag( t.name() ) );
	ensure( tagcoll::utils::set_contains(t.facet().tags(), t) );
}

}

/*
#include <ept/cache/tag.tcc>
#include <ept/cache/debtags/vocabulary.tcc>
*/

// vim:set ts=3 sw=3:
#endif
