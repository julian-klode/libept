/*
 * Merge different vocabularies together and create the tag and facet indexes
 *
 * Copyright (C) 2003-2006  Enrico Zini <enrico@debian.org>
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


#include <ept/debtags/maint/vocabularymerger.h>
#include <ept/debtags/maint/debdbparser.h>

#include <cassert>
#include <cstring>
#include <cstdio>

using namespace std;
using namespace tagcoll;

namespace ept {
namespace debtags {

static void writeDebStyleField(FILE* out, const string& name, const string& val) throw ()
{
	fprintf(out, "%s: ", name.c_str());

	// Properly escape newlines
	bool was_nl = false;
	for (string::const_iterator s = val.begin(); s != val.end(); s++)
		if (was_nl)
			// \n\n -> \n .\n
			if (*s == '\n')
			{
				fputc(' ', out);
				fputc('.', out);
				fputc(*s, out);
			}
			// \n([^ \t]) -> \n \1
			else if (*s != ' ' && *s != '\t')
			{
				fputc(' ', out);
				fputc(*s, out);
				was_nl = false;
			}
			// \n[ \t] goes unchanged
			else
			{
				fputc(*s, out);
				was_nl = false;
			}
		else
			if (*s == '\n')
			{
				fputc(*s, out);
				was_nl = true;
			}
			else
				fputc(*s, out);

	fputc('\n', out);
}

VocabularyMerger::TagData& VocabularyMerger::FacetData::obtainTag(const std::string& name)
{
	std::map<std::string, TagData>::iterator i = tags.find(name);
	if (i == tags.end())
	{
		// Create the tag if it's missing
		pair<std::map<std::string, TagData>::iterator, bool> res = tags.insert(make_pair<std::string, TagData>(name, TagData()));
		i = res.first;
		i->second.name = name;
	}
	return i->second;
}

VocabularyMerger::FacetData& VocabularyMerger::obtainFacet(const std::string& name)
{
	std::map<std::string, FacetData>::iterator i = facets.find(name);
	if (i == facets.end())
	{
		// Create the facet if it's missing
		pair<std::map<std::string, FacetData>::iterator, bool> res = facets.insert(make_pair<std::string, FacetData>(name, FacetData()));
		i = res.first;
		i->second.name = name;
	}
	return i->second;
}

VocabularyMerger::TagData& VocabularyMerger::obtainTag(const std::string& fullname)
{
	size_t p = fullname.find("::");
	if (p == string::npos)
	{
		FacetData& facet = obtainFacet("legacy");
		return facet.obtainTag(fullname);
	} else {
		FacetData& facet = obtainFacet(fullname.substr(0, p));
		return facet.obtainTag(fullname.substr(p + 2));
	}
}


void VocabularyMerger::read(tagcoll::input::Input& input)
{
	DebDBParser parser(input);
	DebDBParser::Record record;

	while (parser.nextRecord(record))
	{
		DebDBParser::Record::const_iterator fi = record.find("Facet");
		DebDBParser::Record::const_iterator ti = record.find("Tag");
		if (fi != record.end())
		{
			// Get the facet record
			FacetData& facet = obtainFacet(fi->second);
			//fprintf(stderr, "Read facet@%d %.*s\n", parser.lineNumber(), PFSTR(facet.name));
			assert(facet.name == fi->second);

			// Merge the data
			for (DebDBParser::Record::const_iterator i = record.begin();
					i != record.end(); i++)
				if (i->first != "Facet")
					facet[i->first] = i->second;
		}
		else if (ti != record.end())
		{
			// Get the tag record
			TagData& tag = obtainTag(ti->second);
			//fprintf(stderr, "Read tag@%d %.*s\n", parser.lineNumber(), PFSTR(tag.name));
			//assert(tag.name == ti->second);

			// Merge the data
			for (DebDBParser::Record::const_iterator i = record.begin();
					i != record.end(); i++)
				if (i->first != "Tag")
					tag[i->first] = i->second;
		}
		else
		{
			fprintf(stderr, "%s:%d: Skipping record without Tag or Facet field\n",
					input.fileName().c_str(), input.lineNumber());
		}
	}
}

bool VocabularyMerger::hasTag(const std::string& fullname) const
{
	size_t p = fullname.find("::");
	std::string facetName;
	std::string tagName;
	if (p == string::npos)
	{
		facetName = "legacy";
		tagName = fullname;
	} else {
		facetName = fullname.substr(0, p);
		tagName = fullname.substr(p + 2);
	}

	std::map<std::string, FacetData>::const_iterator i = facets.find(facetName);
	if (i == facets.end())
		return false;
	return i->second.tags.find(tagName) != i->second.tags.end();
}

int VocabularyMerger::tagID(const std::string& fullname) const
{
	size_t p = fullname.find("::");
	std::string facetName;
	std::string tagName;
	if (p == string::npos)
	{
		facetName = "legacy";
		tagName = fullname;
	} else {
		facetName = fullname.substr(0, p);
		tagName = fullname.substr(p + 2);
	}

	std::map<std::string, FacetData>::const_iterator i = facets.find(facetName);
	if (i == facets.end())
		return -1;
	std::map<std::string, TagData>::const_iterator j = i->second.tags.find(tagName);
	if (j == i->second.tags.end())
		return -1;
	return j->second.id;
}

std::set<std::string> VocabularyMerger::tagNames() const
{
	set<string> res;
	for (std::map<std::string, FacetData>::const_iterator f = facets.begin(); f != facets.end(); f++)
		for (std::map<std::string, TagData>::const_iterator t = f->second.tags.begin();
				t != f->second.tags.end(); t++)
			res.insert(f->first + "::" + t->first);
	return res;
}

void VocabularyMerger::write(const std::string& fname)
{
	FILE* out = fopen(fname.c_str(), "wt");
	if (!out)
		throw wibble::exception::File(fname, "cept_debtags_vocabularymerger:reating file ");
	write(out);
	fclose(out);
}

void VocabularyMerger::write(FILE* out)
{
	long start_ofs = ftell(out);
	int facetid = 0;
	int tagid = 0;

	//fprintf(stderr, "Write\n");
	for (std::map<std::string, FacetData>::iterator f = facets.begin(); f != facets.end(); f++)
	{
		f->second.id = facetid++;
		//fprintf(stderr, "Writing facet %.*s\n", PFSTR(f->first));
		f->second.ofs = ftell(out) - start_ofs;
		writeDebStyleField(out, "Facet", f->first);
		for (std::map<std::string, std::string>::const_iterator j = f->second.begin();
				j != f->second.end(); j++)
			writeDebStyleField(out, j->first, j->second);
		fputc('\n', out);
		f->second.len = ftell(out) - f->second.ofs;

		for (std::map<std::string, TagData>::iterator t = f->second.tags.begin();
				t != f->second.tags.end(); t++)
		{
			t->second.id = tagid++;
			//fprintf(stderr, "Writing tag %.*s\n", PFSTR(t->first));
			t->second.ofs = ftell(out) - start_ofs;
			writeDebStyleField(out, "Tag", f->first + "::" + t->first);
			for (std::map<std::string, std::string>::const_iterator j = t->second.begin();
					j != t->second.end(); j++)
				writeDebStyleField(out, j->first, j->second);
			fputc('\n', out);
			t->second.len = ftell(out) - t->second.ofs;
		}
	}

	tagCount = tagid;
}


int VocabularyMerger::FacetIndexer::encodedSize() const 
{
	// First the main offset table
	int size = vm.facets.size() * sizeof(int);

	for (std::map<std::string, FacetData>::const_iterator f = vm.facets.begin(); f != vm.facets.end(); f++)
	{
		// offset of record in vocabulary
		// size of record in vocabulary
		// id of first tag
		// id of last tag
		// name (0-terminated)
		size += 4 * sizeof(int) + f->first.size() + 1;
		
		// Align to int boundaries
		if ((size % sizeof(int)) != 0)
			size = (size + sizeof(int)) / sizeof(int) * sizeof(int);
	}

	return tagcoll::diskindex::MMap::align(size);
}

void VocabularyMerger::FacetIndexer::encode(char* buf) const 
{
	int pos = vm.facets.size() * sizeof(int);

	for (std::map<std::string, FacetData>::const_iterator f = vm.facets.begin(); f != vm.facets.end(); f++)
	{
		((int*)buf)[f->second.id] = pos;

		// offset of record in vocabulary
		*(int*)(buf+pos) = f->second.ofs;
		pos += sizeof(int);

		// size of record in vocabulary
		*(int*)(buf+pos) = f->second.len;
		pos += sizeof(int);

		if (f->second.tags.empty())
		{
			// id of first tag
			*(int*)(buf+pos) = -1;
			pos += sizeof(int);

			// id of last tag
			*(int*)(buf+pos) = -1;
			pos += sizeof(int);
		} else {
			// id of first tag
			*(int*)(buf+pos) = f->second.tags.begin()->second.id;
			pos += sizeof(int);

			// id of last tag
			*(int*)(buf+pos) = f->second.tags.rbegin()->second.id;
			pos += sizeof(int);
		}

		// name (0-terminated)
		memcpy(buf + pos, f->first.c_str(), f->first.size() + 1);
		pos += f->first.size() + 1;
		
		// Align to int boundaries
		if ((pos % sizeof(int)) != 0)
			pos = (pos + sizeof(int)) / sizeof(int) * sizeof(int);
	}
}

int VocabularyMerger::TagIndexer::encodedSize() const 
{
	// First the main offset table
	int size = vm.tagCount * sizeof(int);

	for (std::map<std::string, FacetData>::const_iterator f = vm.facets.begin(); f != vm.facets.end(); f++)
	{
		for (std::map<std::string, TagData>::const_iterator t = f->second.tags.begin();
				t != f->second.tags.end(); t++)
		{
			// offset of record in vocabulary
			// size of record in vocabulary
			// id of facet
			// name (0-terminated)
			size += 3 * sizeof(int) + f->first.size() + t->first.size() + 3;
		
			// Align to int boundaries
			if ((size % sizeof(int)) != 0)
				size = (size + sizeof(int)) / sizeof(int) * sizeof(int);
		}
	}
	return tagcoll::diskindex::MMap::align(size);
}

void VocabularyMerger::TagIndexer::encode(char* buf) const 
{
	int pos = vm.tagCount * sizeof(int);

	for (std::map<std::string, FacetData>::const_iterator f = vm.facets.begin(); f != vm.facets.end(); f++)
	{
		for (std::map<std::string, TagData>::const_iterator t = f->second.tags.begin();
				t != f->second.tags.end(); t++)
		{
			((int*)buf)[t->second.id] = pos;

			// offset of record in vocabulary
			*(int*)(buf+pos) = t->second.ofs;
			pos += sizeof(int);

			// size of record in vocabulary
			*(int*)(buf+pos) = t->second.len;
			pos += sizeof(int);

			// id of facet
			*(int*)(buf+pos) = f->second.id;
			pos += sizeof(int);

			// name (0-terminated)
			string name = f->first + "::" + t->first;
			memcpy(buf + pos, name.c_str(), name.size() + 1);
			pos += name.size() + 1;
			
			// Align to int boundaries
			if ((pos % sizeof(int)) != 0)
				pos = (pos + sizeof(int)) / sizeof(int) * sizeof(int);
		}
	}
}

}
}

// vim:set ts=4 sw=4:
