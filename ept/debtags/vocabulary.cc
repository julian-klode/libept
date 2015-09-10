/* -*- C++ -*-
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

#include "vocabulary.h"
#include "maint/debdbparser.h"
#include "ept/utils/sys.h"
#include <system_error>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

namespace ept {
namespace debtags {

namespace voc {

std::string getfacet(const std::string& tagname)
{
	size_t p = tagname.find("::");
	if (p == string::npos)
		return "legacy";
	else
		return tagname.substr(0, p);
}

std::string Data::shortDescription() const
{
	if (m_desc.empty())
	{
		string d = longDescription();
		if (d.empty()) return d;
		size_t pos = d.find('\n');
		if (pos == std::string::npos)
			m_desc = d;
		else
			m_desc = d.substr(0, pos);
	}
	return m_desc;
}

std::string Data::longDescription() const
{
	const_iterator i = find("Description");
	if (i == end()) return std::string();
	return i->second;
}

bool FacetData::hasTag(const std::string& name) const
{
	return m_tags.find(name) != m_tags.end();
}

const TagData* FacetData::tagData(const std::string& name) const
{
	std::map<std::string, voc::TagData>::const_iterator i = m_tags.find(name);
	if (i == m_tags.end()) return 0;
	return &i->second;
}

std::set<std::string> FacetData::tags() const
{
	std::set<std::string> res;
	for (std::map<std::string, voc::TagData>::const_iterator i = m_tags.begin();
			i != m_tags.end(); ++i)
		res.insert(i->first);
	return res;
}

}

Vocabulary::Vocabulary(bool empty)
    : m_timestamp(0)
{
    if (empty) return;
    load(pathname());
}

Vocabulary::~Vocabulary()
{
}

string Vocabulary::pathname()
{
    const char* res = getenv("DEBTAGS_VOCABULARY");
    if (!res) res = "/var/lib/debtags/vocabulary";
    return res;
}

void Vocabulary::load(const std::string& pathname)
{
    if (!sys::exists(pathname)) return;
    // Read uncompressed data
    FILE* in = fopen(pathname.c_str(), "rt");
    if (!in)
        throw std::system_error(errno, std::system_category(), "cannot open " + pathname);

    try {
        read(in, pathname);
    } catch (...) {
        fclose(in);
        throw;
    }
    fclose(in);
    m_timestamp = sys::timestamp(pathname, 0);
}

voc::TagData& voc::FacetData::obtainTag(const std::string& name)
{
	std::map<std::string, voc::TagData>::iterator i = m_tags.find(name);
	if (i == m_tags.end())
	{
		// Create the tag if it's missing
		pair<std::map<std::string, TagData>::iterator, bool> res = m_tags.insert(make_pair(name, TagData()));
		i = res.first;
		i->second.name = name;
	}
	return i->second;
}

voc::FacetData& Vocabulary::obtainFacet(const std::string& name)
{
	std::map<std::string, voc::FacetData>::iterator i = m_facets.find(name);
	if (i == m_facets.end())
	{
		// Create the facet if it's missing
		pair<std::map<std::string, voc::FacetData>::iterator, bool> res = m_facets.insert(make_pair(name, voc::FacetData()));
		i = res.first;
		i->second.name = name;
	}
	return i->second;
}

voc::TagData& Vocabulary::obtainTag(const std::string& fullname)
{
	return obtainFacet(voc::getfacet(fullname)).obtainTag(fullname);
}



bool Vocabulary::hasFacet(const std::string& name) const
{
	return m_facets.find(name) != m_facets.end();
}

bool Vocabulary::hasTag(const std::string& name) const
{
	const voc::FacetData* f = facetData(voc::getfacet(name));
	if (!f) return false;
	return f->hasTag(name);
}

const voc::FacetData* Vocabulary::facetData(const std::string& name) const
{
	std::map<std::string, voc::FacetData>::const_iterator i = m_facets.find(name);
	if (i == m_facets.end())
		return 0;
	return &i->second;
}

const voc::TagData* Vocabulary::tagData(const std::string& tagname) const
{
	const voc::FacetData* f = facetData(voc::getfacet(tagname));
	if (!f) return 0;

	return f->tagData(tagname);
}

std::set<std::string> Vocabulary::facets() const
{
	std::set<std::string> res;
	for (std::map<std::string, voc::FacetData>::const_iterator i = m_facets.begin();
			i != m_facets.end(); ++i)
		res.insert(i->first);
	return res;
}

std::set<std::string> Vocabulary::tags() const
{
	std::set<std::string> res;
	for (std::map<std::string, voc::FacetData>::const_iterator i = m_facets.begin();
			i != m_facets.end(); ++i)
		for (std::map<std::string, voc::TagData>::const_iterator j = i->second.m_tags.begin();
				j != i->second.m_tags.end(); ++j)
			res.insert(j->first);
	return res;
}

std::set<std::string> Vocabulary::tags(const std::string& facet) const
{
	const voc::FacetData* f = facetData(facet);
	if (!f) return std::set<std::string>();
	return f->tags();
}

void Vocabulary::read(FILE* input, const std::string& pathname)
{
	DebDBParser parser(input, pathname);
	DebDBParser::Record record;

	while (parser.nextRecord(record))
	{
		DebDBParser::Record::const_iterator fi = record.find("Facet");
		DebDBParser::Record::const_iterator ti = record.find("Tag");
		if (fi != record.end())
		{
			// Get the facet record
			voc::FacetData& facet = obtainFacet(fi->second);
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
			voc::TagData& tag = obtainTag(ti->second);
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
            fprintf(stderr, "%s: Skipping record without Tag or Facet field\n", parser.fileName().c_str());
        }
    }
}

void Vocabulary::write()
{
    string vocfname = pathname();

    // Write out, with appropriate umask
    mode_t prev_umask = umask(022);
    write(vocfname);
    umask(prev_umask);
}

void Vocabulary::write(const std::string& fname)
{
    // Serialize the merged vocabulary data
    std::stringstream str;
    write(str);
    // Write it out atomically
    sys::write_file_atomically(fname, str.str(), 0666);
}

static void writeDebStyleField(std::ostream& out, const string& name, const string& val) throw ()
{
    out << name << ": ";

    // Properly escape newlines
    bool was_nl = false;
    for (string::const_iterator s = val.begin(); s != val.end(); s++)
        if (was_nl)
            // \n\n -> \n .\n
            if (*s == '\n')
                out << " ." << *s;
            // \n([^ \t]) -> \n \1
            else if (*s != ' ' && *s != '\t')
            {
                out << " " << *s;
                was_nl = false;
            }
            // \n[ \t] goes unchanged
            else
            {
                out << *s;
                was_nl = false;
            }
        else
            if (*s == '\n')
            {
                out << *s;
                was_nl = true;
            }
            else
                out << *s;

    out << endl;
}

void Vocabulary::write(std::ostream& out)
{
    for (const auto& f: m_facets)
    {
        //fprintf(stderr, "Writing facet %.*s\n", PFSTR(f->first));
        writeDebStyleField(out, "Facet", f.first);
        for (const auto& j : f.second)
            writeDebStyleField(out, j.first, j.second);
        out << endl;

        for (const auto& t : f.second.m_tags)
        {
            //fprintf(stderr, "Writing tag %.*s\n", PFSTR(t->first));
            writeDebStyleField(out, "Tag", t.first);
            for (const auto& j : t.second)
                writeDebStyleField(out, j.first, j.second);
            out << endl;
        }
    }
}

}
}
