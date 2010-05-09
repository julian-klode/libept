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

#include <ept/debtags/vocabulary.h>
#include <ept/debtags/maint/debdbparser.h>
#include <ept/debtags/maint/path.h>
#include <ept/debtags/maint/sourcedir.h>

#include <tagcoll/input/memory.h>

#include <cstring>
#include <cstdio>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace tagcoll;

namespace ept {
namespace debtags {

static inline std::string getfacet(const std::string& tagname)
{
	size_t p = tagname.find("::");
	if (p == string::npos)
		return "legacy";
	else
		return tagname.substr(0, p);
}

Vocabulary::Vocabulary()
{
	SourceDir mainSource(Path::debtagsSourceDir());
	SourceDir userSource(Path::debtagsUserSourceDir());

	mainSource.readVocabularies(*this);
	userSource.readVocabularies(*this);

	time_t ts_main_src = mainSource.vocTimestamp();
	time_t ts_user_src = userSource.vocTimestamp();
	m_timestamp = ts_main_src > ts_user_src ? ts_main_src : ts_user_src;
}

Vocabulary::~Vocabulary()
{
}

voc::TagData& voc::FacetData::obtainTag(const std::string& name)
{
	std::map<std::string, voc::TagData>::iterator i = tags.find(name);
	if (i == tags.end())
	{
		// Create the tag if it's missing
		pair<std::map<std::string, TagData>::iterator, bool> res = tags.insert(make_pair<std::string, TagData>(name, TagData()));
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
		pair<std::map<std::string, voc::FacetData>::iterator, bool> res = m_facets.insert(make_pair<std::string, voc::FacetData>(name, voc::FacetData()));
		i = res.first;
		i->second.name = name;
	}
	return i->second;
}

voc::TagData& Vocabulary::obtainTag(const std::string& fullname)
{
	size_t p = fullname.find("::");
	if (p == string::npos)
	{
		voc::FacetData& facet = obtainFacet("legacy");
		return facet.obtainTag(fullname);
	} else {
		voc::FacetData& facet = obtainFacet(fullname.substr(0, p));
		return facet.obtainTag(fullname.substr(p + 2));
	}
}



bool Vocabulary::hasFacet(const std::string& name) const
{
	return m_facets.find(name) != m_facets.end();
}

bool Vocabulary::hasTag(const std::string& name) const
{
	const voc::FacetData* f = facetData(getfacet(name));
	if (!f) return false;
	return f->tags.find(name) != f->tags.end();
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
	const voc::FacetData* f = facetData(getfacet(tagname));
	if (!f) return 0;

	std::map<std::string, voc::TagData>::const_iterator i = f->tags.find(tagname);
	if (i == f->tags.end()) return 0;

	return &i->second;
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
		for (std::map<std::string, voc::TagData>::const_iterator j = i->second.tags.begin();
				j != i->second.tags.end(); ++j)
			res.insert(j->first);
	return res;
}

std::set<std::string> Vocabulary::tags(const std::string& facet) const
{
	std::set<std::string> res;
	const voc::FacetData* f = facetData(facet);
	if (!f) return res;
	for (std::map<std::string, voc::TagData>::const_iterator i = f->tags.begin();
			i != f->tags.end(); ++i)
		res.insert(i->first);
	return res;
}

void Vocabulary::read(tagcoll::input::Input& input)
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
			fprintf(stderr, "%s:%d: Skipping record without Tag or Facet field\n",
					input.fileName().c_str(), input.lineNumber());
		}
	}
}

void Vocabulary::write(const std::string& fname)
{
	// Build the temp file template
	char tmpfname[fname.size() + 7];
	strncpy(tmpfname, fname.c_str(), fname.size());
	strncpy(tmpfname + fname.size(), ".XXXXXX", 8);

	// Create and open the temporary file
	int fd = mkstemp(tmpfname);
	if (fd < 0)
		throw wibble::exception::File(tmpfname, "opening file");

	// Read the current umask
	mode_t cur_umask = umask(0);
	umask(cur_umask);

	// Give the file the right permissions
	if (fchmod(fd, 0666 & ~cur_umask) < 0)
		throw wibble::exception::File(tmpfname, "setting file permissions");

	// Pass the file descriptor to stdio
	FILE* out = fdopen(fd, "wt");
	if (!out)
		throw wibble::exception::File(tmpfname, "fdopening file");

	// Write out the merged vocabulary data
	write(out);

	// Flush stdio's buffers
	fflush(out);

	// Flush OS buffers
	fdatasync(fd);

	// Close the file
	fclose(out);

	// Rename the successfully written file to its final name
	if (rename(tmpfname, fname.c_str()) == -1)
		throw wibble::exception::System(string("renaming ") + tmpfname + " to " + fname);
}

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

void Vocabulary::write(FILE* out)
{
	long start_ofs = ftell(out);
	int facetid = 0;
	int tagid = 0;

	//fprintf(stderr, "Write\n");
	for (std::map<std::string, voc::FacetData>::iterator f = m_facets.begin(); f != m_facets.end(); ++f)
	{
		//fprintf(stderr, "Writing facet %.*s\n", PFSTR(f->first));
		writeDebStyleField(out, "Facet", f->first);
		for (std::map<std::string, std::string>::const_iterator j = f->second.begin();
				j != f->second.end(); j++)
			writeDebStyleField(out, j->first, j->second);
		fputc('\n', out);

		for (std::map<std::string, voc::TagData>::iterator t = f->second.tags.begin();
				t != f->second.tags.end(); t++)
		{
			//fprintf(stderr, "Writing tag %.*s\n", PFSTR(t->first));
			writeDebStyleField(out, "Tag", f->first + "::" + t->first);
			for (std::map<std::string, std::string>::const_iterator j = t->second.begin();
					j != t->second.end(); j++)
				writeDebStyleField(out, j->first, j->second);
			fputc('\n', out);
		}
	}
}

}
}

// vim:set ts=4 sw=4:
