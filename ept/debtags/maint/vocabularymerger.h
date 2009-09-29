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

#include <tagcoll/diskindex/mmap.h>
#include <tagcoll/input/base.h>
#include <string>
#include <map>
#include <set>

#ifndef EPT_DEBTAGS_VOCABULARYMERGER_H
#define EPT_DEBTAGS_VOCABULARYMERGER_H

namespace ept {
namespace debtags {

class VocabularyMerger
{
protected:
	class FacetIndexer : public tagcoll::diskindex::MMapIndexer
	{
	protected:
		VocabularyMerger& vm;
	public:
		FacetIndexer(VocabularyMerger& vm) : vm(vm) {}
		virtual ~FacetIndexer() {}
		virtual int encodedSize() const;
		virtual void encode(char* buf) const;
	};
	class TagIndexer : public tagcoll::diskindex::MMapIndexer
	{
	protected:
		VocabularyMerger& vm;
	public:
		TagIndexer(VocabularyMerger& vm) : vm(vm) {}
		virtual ~TagIndexer() {}
		virtual int encodedSize() const;
		virtual void encode(char* buf) const;
	};
	class TagData : public std::map<std::string, std::string>
	{
	public:
		std::string name;
		// Offset in the last written file (used for indexing)
		long ofs;
		int len;
		int id;

		TagData() : ofs(0), len(0) {}
	};
	class FacetData : public std::map<std::string, std::string>
	{
	public:
		std::string name;
		std::map<std::string, TagData> tags;
		// Offset in the last written file (used for indexing)
		long ofs;
		int len;
		int id;

		FacetData() : ofs(0), len(0) {}

		TagData& obtainTag(const std::string& fullname);
	};
	std::map<std::string, FacetData> facets;
	int tagCount;
	FacetIndexer findexer;
	TagIndexer tindexer;
	
	FacetData& obtainFacet(const std::string& name);
	TagData& obtainTag(const std::string& fullname);
	
public:
	VocabularyMerger() : tagCount(0), findexer(*this), tindexer(*this) {}

	/**
	 * Check if there is any data in the merged vocabulary
	 */
	bool empty() const { return facets.empty(); }

	/**
	 * Parse and import the vocabulary from `input', merging the data with the
	 * previously imported ones
	 */
	void read(tagcoll::input::Input& input);

	/**
	 * Write the vocabulary data to the given file
	 */
	void write(const std::string& fname);

	/**
	 * Write the vocabulary data to the given output stream
	 */
	void write(FILE* out);

	/**
	 * Get the facet indexer.
	 *
	 * Note: the indexers will only be functional after one of the write
	 * methods have been invoked
	 */
	const tagcoll::diskindex::MMapIndexer& facetIndexer() const { return findexer; }

	/**
	 * Get the tag indexer.
	 *
	 * Note: the indexers will only be functional after one of the write
	 * methods have been invoked
	 */
	const tagcoll::diskindex::MMapIndexer& tagIndexer() const { return tindexer; }

	/**
	 * Check if the vocabulary contains the facet `name'
	 */
	bool hasFacet(const std::string& name) const
	{
		return facets.find(name) != facets.end();
	}

	/**
	 * Check if the vocabulary contains the tag `fullname'
	 */
	bool hasTag(const std::string& fullname) const;

	/**
	 * Return the ID for the given tag (or -1 if not found)
	 */
	int tagID(const std::string& fullname) const;

	/**
	 * Return a set with all tag names
	 */
	std::set<std::string> tagNames() const;
};

}
}

// vim:set ts=4 sw=4:
#endif
