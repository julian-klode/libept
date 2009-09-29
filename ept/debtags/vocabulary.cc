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
#include <ept/debtags/maint/vocabularyindexer.h>
#include <ept/debtags/maint/debdbparser.h>
#include <ept/debtags/maint/path.h>

#include <tagcoll/input/memory.h>

#include <cstring>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

using namespace tagcoll;

namespace ept {
namespace debtags {

int Vocabulary::FacetIndex::id(const char* name) const
{
	if (size() == 0) return -1;
	int begin, end;

	/* Binary search */
	begin = -1, end = size();
	while (end - begin > 1)
	{
		int cur = (end + begin) / 2;
		if (strcmp(item(cur)->name, name) > 0)
			end = cur;
		else
			begin = cur;
	}

	if (begin == -1 || strcmp(item(begin)->name, name) != 0)
		//throw NotFoundException(string("looking for the ID of string ") + str);
		return -1;
	else
		return begin;
}

int tagcmp(const char* tag1, const char* tag2)
{
	const char* tsep1 = strstr(tag1, "::");
	if (tsep1 == NULL) return strcmp(tag1, tag2);
	const char* tsep2 = strstr(tag2, "::");
	if (tsep2 == NULL) return strcmp(tag1, tag2);

	// See what is the length of the shortest facet
	int len1 = tsep1 - tag1;
	int len2 = tsep2 - tag2;
	int minlen = len1 < len2 ? len1 : len2;

	int res = strncmp(tag1, tag2, minlen);
	if (res != 0)
		// Different facets
		return res;

	if (len1 == len2)
		// If the facet is the same, compare the tags
		return strcmp(tsep1 + 2, tsep2 + 2);
	else
		// Two facets with similar prefixes
		return len1 < len2 ? -1 : 1;
}

int Vocabulary::TagIndex::id(const char* name) const
{
	if (size() == 0) return -1;
	int begin, end;

	/* Binary search */
	begin = -1, end = size();
	while (end - begin > 1)
	{
		int cur = (end + begin) / 2;
		if (tagcmp(item(cur)->name, name) > 0)
			end = cur;
		else
			begin = cur;
	}

	if (begin == -1 || tagcmp(item(begin)->name, name) != 0)
		//throw NotFoundException(string("looking for the ID of string ") + str);
		return -1;
	else
		return begin;
}

Vocabulary::Vocabulary()
	: voc_fd(-1), voc_size(0), voc_buf(0)
{
	std::string vocfname;
	std::string idxfname;

	if (!VocabularyIndexer::obtainWorkingVocabulary(vocfname, idxfname))
	{
		m_timestamp = 0;
		return;
	}

	m_timestamp = Path::timestamp(idxfname);

	mastermmap.init(idxfname);

	// Initialize the facet and tag indexes
	findex.init(mastermmap, 0);
	tindex.init(mastermmap, 1);

	// MMap the vocabulary

	// Open the file
	voc_fname = vocfname;
	if ((voc_fd = open(voc_fname.c_str(), O_RDONLY)) == -1)
		throw wibble::exception::File(voc_fname, "opening vocabulary file");

	off_t size = lseek(voc_fd, 0, SEEK_END);
	if (size == (off_t)-1)
		throw wibble::exception::File(voc_fname, "reading the size of vocabulary file");
	voc_size = size;

	// Map the file into memory
	if ((voc_buf = (const char*)mmap(0, voc_size, PROT_READ, MAP_PRIVATE, voc_fd, 0)) == MAP_FAILED)
		throw wibble::exception::File(voc_fname, "mmapping vocabulary file");
}

Vocabulary::~Vocabulary()
{
	// Unmap and close the file
	if (voc_buf)
		munmap((void*)voc_buf, voc_size);
	if (voc_fd != -1)
		close(voc_fd);
}

Facet Vocabulary::facetByID(int id) const
{
	return Facet(this, id);
}

Tag Vocabulary::tagByID(int id) const
{
	return Tag(this, id);
}

void Vocabulary::parseVocBuf(std::map<std::string, std::string>& res, size_t ofs, size_t len) const
{
	// Access the right part of the mmapped buffer
    std::stringstream name;
	name << voc_fname << '+' << ofs << '-' << len;
	input::Memory in(name.str(), voc_buf + ofs, len);
	DebDBParser parser(in);
	// Parse the raw string data and store it in the cache vector
	parser.nextRecord(res);

    std::string desc = res["Description"];
	if (!desc.empty())
	{
		size_t pos = desc.find('\n');
		if (pos == std::string::npos)
			res["_SD_"] = desc;
		else
			res["_SD_"] = desc.substr(0, pos);
	}
}

std::string Vocabulary::tagShortName(int id) const
{
	const char* fullname = tindex.name(id);
	const char* sub = strstr(fullname, "::");
	if (sub != NULL)
		return sub + 2;
	else
		return fullname;
}

const std::map<std::string, std::string>& Vocabulary::facetData(int id) const
{
	if (id < 0) return emptyData;

	// Enlarge the cache vector if needed
	if ((unsigned)id >= m_facetData.size())
		m_facetData.resize(id + 1);

	if (m_facetData[id].empty())
		parseVocBuf(m_facetData[id], findex.offset(id), findex.size(id));

	return m_facetData[id];
}

const std::map<std::string, std::string>& Vocabulary::tagData(int id) const
{
	if (id < 0) return emptyData;

	// Enlarge the cache vector if needed
	if ((unsigned)id >= m_tagData.size())
		m_tagData.resize(id + 1);

	if (m_tagData[id].empty())
		parseVocBuf(m_tagData[id], tindex.offset(id), tindex.size(id));

	return m_tagData[id];
}

}
}

// vim:set ts=4 sw=4:
