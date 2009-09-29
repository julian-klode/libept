/*
 * Merge different vocabularies together and create the tag and facet indexes
 *
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

#include <wibble/test.h>
#include <ept/debtags/maint/vocabularymerger.h>
#include <tagcoll/input/string.h>

using namespace std;
using namespace tagcoll;

struct TestVocabularyMerger {

    inline static const char* indexref(const char* index, int id)
    {
	return index + ((int*)index)[id];
    }


    Test _1()
{
	string voc1 =
		"Facet: taste\n"
		"Description: Taste\n\n"
		"Tag: taste::sweet\n"
		"Description: Sweet\n\n"
		"Tag: taste::salty\n"
		"Description: Salty\n\n";
	string voc2 =
		"Facet: smell\n"
		"Description: Smell\n\n"
		"Tag: smell::fresh\n"
		"Description: Fresh\n\n"
		"Tag: smell::mold\n"
		"Description: Mold\n\n";
	tagcoll::input::String in1(voc1);
	tagcoll::input::String in2(voc2);

	ept::debtags::VocabularyMerger vm;

	// Read and merge the two vocabulary samples
	vm.read(in1);
	vm.read(in2);

	// Write the merged vocabulary to /dev/null (but generate offsets and indexes in the meantime)
	vm.write("/dev/null");

	// Create the facet index
	char facetIndex[vm.facetIndexer().encodedSize()];
	vm.facetIndexer().encode(facetIndex);

	// Create the tag index
	char tagIndex[vm.tagIndexer().encodedSize()];
	vm.tagIndexer().encode(tagIndex);

	// Check that the facet names have been encoded correctly and in order
	assert_eq(string(indexref(facetIndex, 0) + 4*sizeof(int)), "smell");
	assert_eq(string(indexref(facetIndex, 1) + 4*sizeof(int)), "taste");

	// Check the first and last tag indexes for the facets
	assert_eq(((int*)indexref(facetIndex, 0))[2], 0);
	assert_eq(((int*)indexref(facetIndex, 0))[3], 1);
	assert_eq(((int*)indexref(facetIndex, 1))[2], 2);
	assert_eq(((int*)indexref(facetIndex, 1))[3], 3);

	// Check that the tag names have been encoded correctly and in order
	assert_eq(string(indexref(tagIndex, 0) + 3*sizeof(int)), "smell::fresh");
	assert_eq(string(indexref(tagIndex, 1) + 3*sizeof(int)), "smell::mold");
	assert_eq(string(indexref(tagIndex, 2) + 3*sizeof(int)), "taste::salty");
	assert_eq(string(indexref(tagIndex, 3) + 3*sizeof(int)), "taste::sweet");

	// Check the facet indexes for the tags
	assert_eq(((int*)indexref(tagIndex, 0))[2], 0);
	assert_eq(((int*)indexref(tagIndex, 1))[2], 0);
	assert_eq(((int*)indexref(tagIndex, 2))[2], 1);
	assert_eq(((int*)indexref(tagIndex, 3))[2], 1);
}

// Test parsing a vocabulary with a tag without a defined facet
    Test _2()
{
	string voc =
		"Tag: foo::bar\n"
		"Description: Tag without facet\n"
		" VocabularyMerged should behave fine in this case.\n\n";
	tagcoll::input::String in(voc);

	ept::debtags::VocabularyMerger vm;
	vm.read(in);

	// Write the merged vocabulary to /dev/null (but generate offsets and indexes in the meantime)
	vm.write("/dev/null");

	// Create the facet index
	char facetIndex[vm.facetIndexer().encodedSize()];
	vm.facetIndexer().encode(facetIndex);

	// Create the tag index
	char tagIndex[vm.tagIndexer().encodedSize()];
	vm.tagIndexer().encode(tagIndex);
}

// Test parsing a vocabulary with a facet without tags
    Test _3()
{
	string voc =
		"Facet: empty\n"
		"Description: Facet without tags\n"
		" VocabularyMerged used to segfault in this case.\n\n";
	tagcoll::input::String in(voc);

	ept::debtags::VocabularyMerger vm;
	vm.read(in);

	// Write the merged vocabulary to /dev/null (but generate offsets and indexes in the meantime)
	vm.write("/dev/null");

	// Create the facet index
	char facetIndex[vm.facetIndexer().encodedSize()];
	vm.facetIndexer().encode(facetIndex);

	// Create the tag index
	char tagIndex[vm.tagIndexer().encodedSize()];
	vm.tagIndexer().encode(tagIndex);
}

};
// vim:set ts=4 sw=4:
