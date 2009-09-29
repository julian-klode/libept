#ifndef EPT_DEBTAGS_VOCABULARY_H
#define EPT_DEBTAGS_VOCABULARY_H

/** @file
 * @author Enrico Zini <enrico@enricozini.org>
 * Debtags vocabulary access
 */

/*
 * Copyright (C) 2003,2004,2005,2006,2007  Enrico Zini <enrico@debian.org>
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

#include <ept/debtags/tag.h>
#include <tagcoll/diskindex/mmap.h>

#include <string>
#include <vector>
#include <map>

namespace ept {
namespace debtags {

class Vocabulary
{
public:
	class FacetIndex : public tagcoll::diskindex::MMap
	{
	protected:
		// Layout of the data in the index
		struct Item {
			int offset;
			int size;
			int firsttag;
			int lasttag;
			const char name[];
		};
		inline Item* item(int id) const
		{
			if (id >= 0 && (unsigned)id < size())
				return (Item*)(m_buf + ((int*)m_buf)[id]);
			return NULL;
		}

	public:
		FacetIndex() : tagcoll::diskindex::MMap() {}
		FacetIndex(const tagcoll::diskindex::MasterMMap& master, size_t idx)
			: tagcoll::diskindex::MMap(master, idx) {}

		/// Get the number of facets in the index
		size_t size() const { return m_size == 0 ? 0 :  *(int*)m_buf / sizeof(int); }
		/// Get the offset of the facet data in the vocabulary for this facet
		size_t offset(int id) const { Item* i = item(id); return i == NULL ? 0 : i->offset; }
		/// Get the size of the facet data in the vocabulary for this facet
		size_t size(int id) const { Item* i = item(id); return i == NULL ? 0 : i->size; }
		/// Get the id of the first tag for this facet
		int firsttag(int id) const { Item* i = item(id); return i == NULL ? -1 : i->firsttag; }
		/// Get the id of the last tag for this facet
		int lasttag(int id) const { Item* i = item(id); return i == NULL ? -1 : i->lasttag; }
		/// Get the name of this facet
		const char* name(int id) const { Item* i = item(id); return i == NULL ? "" : i->name; }
		/// Get the ID of the facet with this name
		int id(const char* name) const;
		int id(const std::string& name) const { return id(name.c_str()); }
	};
	
	class TagIndex : public tagcoll::diskindex::MMap
	{
	protected:
		// Layout of the data in the index
		struct Item {
			int offset;
			int size;
			int facet;
			const char name[];
		};
		inline Item* item(int id) const
		{
			if (id >= 0 && (unsigned)id < size())
				return (Item*)(m_buf + ((int*)m_buf)[id]);
			return NULL;
		}

	public:
		TagIndex() : tagcoll::diskindex::MMap() {}
		TagIndex(const tagcoll::diskindex::MasterMMap& master, size_t idx)
			: tagcoll::diskindex::MMap(master, idx) {}

		/// Get the number of tags in the index
		size_t size() const { return m_size == 0 ? 0 : *(int*)m_buf / sizeof(int); }
		/// Get the offset of the facet data in the vocabulary for this tag
		size_t offset(int id) const { Item* i = item(id); return i == NULL ? 0 : i->offset; }
		/// Get the size of the facet data in the vocabulary for this tag
		size_t size(int id) const { Item* i = item(id); return i == NULL ? 0 : i->size; }
		/// Get the id of the facet for this tag
		int facet(int id) const { Item* i = item(id); return i == NULL ? -1 : i->facet; }
		/// Get the name of this tag
		const char* name(int id) const { Item* i = item(id); return i == NULL ? "" : i->name; }
		/// Get the ID of the tag with this name
		int id(const char* name) const;
		int id(const std::string& name) const { return id(name.c_str()); }
	};

protected:
	// Master MMap index container
	tagcoll::diskindex::MasterMMap mastermmap;

	time_t m_timestamp;

	// Mmapped vocabulary file
	std::string voc_fname;
	int voc_fd;
	size_t voc_size;
	const char* voc_buf;
	
	// Facet and tag indexes
	FacetIndex findex;
	TagIndex tindex;
	
	// Cached parsed facet and tag records
	mutable std::vector< std::map<std::string, std::string> > m_facetData;
	mutable std::vector< std::map<std::string, std::string> > m_tagData;
	// Empty parsed data to return when data is asked for IDs == -1
	std::map<std::string, std::string> emptyData;

	void parseVocBuf(std::map<std::string, std::string>& res, size_t ofs, size_t len) const;

public:
	Vocabulary();
	~Vocabulary();

	/// Get the timestamp of when the index was last updated
	time_t timestamp() const { return m_timestamp; }

	/// Return true if this data source has data, false if it's empty
	bool hasData() const { return m_timestamp != 0; }

	const FacetIndex& facetIndex() const { return findex; }
	const TagIndex& tagIndex() const { return tindex; }

	/**
	 * Check if the vocabulary contains the facet `name'
	 */
	bool hasFacet(const std::string& name) const
	{
		return findex.id(name.c_str()) != -1;
	}

	/**
	 * Check if the vocabulary contains the tag `fullname'
	 */
	bool hasTag(const std::string& fullname) const
	{
		return tindex.id(fullname.c_str()) != -1;
	}

	/**
	 * Return the facet with the given name
	 */
	Facet facetByID(int id) const;

	/**
	 * Return the tag with the given full name
	 */
	Tag tagByID(int id) const;

	template<typename IDS>
	std::set<Tag> tagsByID(const IDS& ids) const
	{
		std::set<Tag> res;
		for (typename IDS::const_iterator i = ids.begin();
				i != ids.end(); ++i)
			res.insert(tagByID(*i));
		return res;
	}

	/**
	 * Return the facet for the tag with the given ID
	 */
	Facet facetByTag(int id) const { return facetByID(tindex.facet(id)); }

	/**
	 * Return the facet with the given name
	 */
	Facet facetByName(const std::string& name) const { return facetByID(findex.id(name)); }

	/**
	 * Return the tag with the given full name
	 */
	Tag tagByName(const std::string& fullname) const { return tagByID(tindex.id(fullname)); }

	/**
	 * Return all the facets in the vocabulary
	 */
	std::set< Facet > facets() const
	{
		std::set< Facet > res;
		for (size_t i = 0; i < findex.size(); i++)
			res.insert(facetByID(i));
		return res;
	}

	/**
	 * Return all the tags in the vocabulary
	 */
	std::set< Tag > tags() const
	{
		std::set< Tag > res;
		for (size_t i = 0; i < tindex.size(); i++)
			res.insert(tagByID(i));
		return res;
	}

	/**
	 * Return the tags in the given facet
	 */
	std::set< Tag > tags(int facet) const
	{
		std::set< Tag > res;
		for (int i = findex.firsttag(facet); i != -1 && i <= findex.lasttag(facet); i++)
			res.insert(tagByID(i));
		return res;
	}

	std::set< Tag > tags(const std::string& facetName) const
	{
		return tags(findex.id(facetName));
	}

	std::set< Tag > tags(const Facet& facet) const
	{
		return tags(facet.id());
	}

#if 0
	/// Get the DerivedTagList with the Equates: expressions read from the vocabulary
	const DerivedTagList& getEquations() const throw () { return equations; }
	
	/// Get a set with all the facets present in the vocabulary that are matched by `filter'
	FacetSet facets(const FacetMatcher& filter) const throw () { return getFiltered(filter); }
#endif

#if 0
	// These functions are here just to be used by Facet and Tag.  I'm not
	// making them private because I don't want Facet and Tag to access other
	// Vocabulary member, and other classes can't use these anyway as Facet::Data and
	// Tag::Data are protected
	const Facet::Data& facetData(int idx) { return m_facets[idx]; }
	const Tag::Data& tagData(int idx) { return m_tags[idx]; }
#endif

	/// Get the facet name given the facet id
	std::string facetName(int id) const { return findex.name(id); }

	/// Get the tag name given the tag id
	std::string tagName(int id) const { return tindex.name(id); }

	/// Get the tag name given the tag id
	std::string tagShortName(int id) const;

	const std::map<std::string, std::string>& facetData(int id) const;
	const std::map<std::string, std::string>& tagData(int id) const;
};

}
}

// vim:set ts=4 sw=4:
#endif
