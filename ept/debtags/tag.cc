/* -*- C++ -*-
 * Copyright (C) 2005,2006  Enrico Zini <enrico@debian.org>
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
#include <ept/debtags/vocabulary.h>
#include <stdexcept>

namespace ept {
namespace debtags {

static inline std::string constget(const std::map<std::string, std::string>& m,
                                   const std::string& key)
{
	std::map<std::string, std::string>::const_iterator i = m.find(key);
	if (i == m.end())
		return std::string();
	else
		return i->second;
}

std::string Facet::name() const
{
	if (valid())
		return m_tags->facetName(m_id);
	throw std::out_of_range( "No name for this facet" );
}
std::string Facet::name(const std::string& d) const
{
	return valid() ? m_tags->facetName(m_id) : d;
}

std::string Facet::shortDescription() const
{
	if (valid())
		return constget(m_tags->facetData(m_id), "_SD_");
	throw std::out_of_range( "No short description for this facet" );
}
std::string Facet::shortDescription(const std::string& d) const
{
	return valid() ? constget(m_tags->facetData(m_id), "_SD_") : d;
}

std::string Facet::longDescription() const
{
	if (valid())
		return constget(m_tags->facetData(m_id), "Description");
	throw std::out_of_range( "No long description for this facet" );
}
std::string Facet::longDescription(const std::string& d) const
{
	return valid() ? constget(m_tags->facetData(m_id), "Description") : d;
}

bool Facet::hasTag(const std::string& name) const
{
	if (!valid())
		throw std::out_of_range( "hasTag() called on an invalid facet" );
	return m_tags->hasTag(this->name() + "::" + name);
}

std::set< Tag > Facet::tags() const
{
	if (!valid())
		throw std::out_of_range( "tags() called on an invalid facet" );
	return m_tags->tags(m_id);
}


Facet Tag::facet() const
{
	if (valid())
		return m_tags->facetByTag(m_id);
	throw std::out_of_range( "No facet for this tag" );
}

std::string Tag::name() const
{
	if (valid())
		return m_tags->tagShortName(m_id);
	throw std::out_of_range( "No name for this tag" );
}
std::string Tag::name(const std::string& d) const
{
	return valid() ? m_tags->tagShortName(m_id) : d;
}

std::string Tag::fullname() const
{
	if (valid())
		return m_tags->tagName(m_id);
	throw std::out_of_range( "No full name for this tag" );
}
std::string Tag::fullname(const std::string& d) const
{
	return valid() ? m_tags->tagName(m_id) : d;
}

std::string Tag::shortDescription() const
{
	if (valid())
		return constget(m_tags->tagData(m_id), "_SD_");
	throw std::out_of_range( "No short description for this tag" );
}
std::string Tag::shortDescription(const std::string& d) const
{
	return valid() ? constget(m_tags->tagData(m_id), "_SD_") : d;
}

std::string Tag::longDescription() const
{
	if (valid())
		return constget(m_tags->tagData(m_id), "Description");
	throw std::out_of_range( "No long description for this tag" );
}
std::string Tag::longDescription(const std::string& d) const
{
	return valid() ? constget(m_tags->tagData(m_id), "Description") : d;
}

}
}

// vim:set ts=3 sw=3:
