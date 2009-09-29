// -*- C++ -*-
#ifndef EPT_DEBTAGS_TAG_H
#define EPT_DEBTAGS_TAG_H

/** \file
 * Debtags facets and tags
 */

/*
 * Copyright (C) 2005,2006,2007  Enrico Zini <enrico@debian.org>
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

#include <set>
#include <string>

namespace ept {
namespace debtags {

class Vocabulary;

class Tag;

/**
 * Representation of a facet.
 *
 * ept::debtags::Facet represents a Facet with all its informations.
 * It is guaranteed to have fast value-copy semantics, so it can be passed
 * around freely and efficiently without worrying about memory management
 * issues.
 *
 * The class is normally instantiated using a Vocabulary:
 * \code
 * 	Facet facet = vocabulary.faceByName("made-of");
 * \endcode
 *
 * Facets can contain an "invalid" value, in which case using any of their
 * methods will likely produce segfault.  The "invalid" facets are useful as
 * "none" return values:
 *
 * \code
 *    Facet facet = vocabulary.facetByName("made-of");
 *    if (!facet)
 *       throw SomeException("facet \"made-of\" has not been defined");
 * \endcode
 */
class Facet
{
protected:
	const Vocabulary* m_tags;
	int m_id;

	Facet(const Vocabulary* tags, int id) : m_tags(tags), m_id(id) {}

public:
	Facet() : m_tags(0), m_id(-1) {}
	~Facet() {}

	bool operator==(const Facet& f) const { return m_id == f.m_id; }
	bool operator!=(const Facet& f) const { return m_id != f.m_id; }
	bool operator<(const Facet& f) const { return m_id < f.m_id; }

	/**
	 * Return true if the facet is valid
	 */
	operator bool() const { return m_id != -1; }
	bool valid() const { return m_id != -1; }

	/**
	 * Return the name of the facet
	 * @throws std::out_of_range if the facet is not valid
	 */
	std::string name() const;
	/**
	 * Return the name of the facet
	 * 
	 * Returns d if the facet is not valid.
	 */
	std::string name(const std::string& d) const;

	/**
	 * Return the short description of the facet
	 * @throws std::out_of_range if the facet is not valid
	 */
	std::string shortDescription() const;
	/**
	 * Return the short description of the facet
	 *
	 * Returns d if the facet is not valid.
	 */
	std::string shortDescription(const std::string& d) const;

	/**
	 * Return the long description of the facet
	 * @throws std::out_of_range if the facet is not valid
	 */
	std::string longDescription() const;
	/**
	 * Return the long description of the facet
	 *
	 * Returns d if the facet is not valid.
	 */
	std::string longDescription(const std::string& d) const;

	/**
	 * Return true if the facet has a tag with the given name (name, not fullname)
	 */
	bool hasTag(const std::string& name) const;

	/**
	 * Return the list of tags in this facet
	 */
	std::set<Tag> tags() const;

	/**
	 * Return the ID of this facet
	 *
	 * @warning This method is exported to help in writing tests, but it is not
	 * part of the normal API: do not use it, because future implementations may
	 * not be based on IDs and therefore not have this method.
	 */
	int id() const { return m_id; }

	friend class Vocabulary;
};

/**
 * Representation of a tag.
 *
 * ept::debtags::Tag represents a Tag with all its informations.
 * It is guaranteed to have fast value-copy semantics, so it can be passed
 * around freely and efficiently without worrying about memory management
 * issues.
 *
 * The class is normally instantiated using a Vocabulary:
 * \code
 * 	Tag tag = vocabulary.tagByName("made-of::lang:c++");
 * \endcode
 *
 * Tags can contain an "invalid" value, in which case using any of their
 * methods will likely produce segfault.  The "invalid" facets are useful as
 * "none" return values:
 *
 * \code
 *    Tag tag = vocabulary.tagByName("made-of");
 *    if (!tag)
 *       throw SomeException("tag \"mytag\" has not been defined");
 * \endcode
 */
class Tag
{
protected:
	const Vocabulary* m_tags;
	int m_id;

	Tag(const Vocabulary* tags, int id) : m_tags(tags), m_id(id) {}

public:
	typedef std::set< Tag > Set;

	Tag() : m_tags(0), m_id(-1) {}
	~Tag() {}

	bool operator==(const Tag& f) const { return m_id == f.m_id; }
	bool operator!=(const Tag& f) const { return m_id != f.m_id; }
	bool operator<(const Tag& f) const { return m_id < f.m_id; }

	operator bool() const { return m_id != -1; }
	bool valid() const { return m_id != -1; }

	Facet facet() const;

	/**
	 * Return the name of the tag, without the facet:: prefix
	 * @throws std::out_of_range if the tag is not valid
	 */
	std::string name() const;
	/**
	 * Return the short description of the tag
	 *
	 * Returns d if the tag is not valid.
	 */
	std::string name(const std::string& d) const;

	/**
	 * Return the name of the tag, with the facet:: prefix
	 * @throws std::out_of_range if the tag is not valid
	 */
	std::string fullname() const;
	/**
	 * Return the short description of the tag
	 *
	 * Returns d if the tag is not valid.
	 */
	std::string fullname(const std::string& d) const;

	/**
	 * Return the short description of the tag
	 * @throws std::out_of_range if the tag is not valid
	 */
	std::string shortDescription() const;
	/**
	 * Return the short description of the tag
	 *
	 * Returns d if the tag is not valid.
	 */
	std::string shortDescription(const std::string& d) const;

	/**
	 * Return the long description of the tag
	 *
	 * @throws std::out_of_range if the tag is not valid
	 */
	std::string longDescription() const;
	/**
	 * Return the long description of the tag
	 *
	 * Returns d if the tag is not valid.
	 */
	std::string longDescription(const std::string& d) const;

	/**
	 * Return the ID of this tag
	 *
	 * @warning This method is exported to help in writing tests, but it is not
	 * part of the normal API: do not use it, because future implementations may
	 * not be based on IDs and therefore not have this method.
	 */
	int id() const { return m_id; }

	friend class Vocabulary;
};

}
}

// vim:set ts=3 sw=3:
#endif
