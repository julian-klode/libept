// -*- mode: c++; tab-width: 4; indent-tabs-mode: t -*-
#ifndef EPT_DEBTAGS_PKGID_H
#define EPT_DEBTAGS_PKGID_H

/** @file
 * @author Enrico Zini <enrico@enricozini.org>
 * Quick map from package IDs to package names
 */

/*
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
#include <string>

namespace ept {
namespace debtags {

/**
 * Maps Packages to IDs and vice-versa.
 *
 * This is used in building the Debtags fast index, which works representing
 * tags and packages as int IDs
 */
class PkgId : public tagcoll::diskindex::MMap
{
	tagcoll::diskindex::MasterMMap mastermmap;
	time_t m_timestamp;

public:
	PkgId();
	PkgId(const tagcoll::diskindex::MasterMMap& master, size_t idx);
	PkgId(const char* buf, int size);

	/// Get the timestamp of when the index was last updated
	time_t timestamp() const { return m_timestamp; }

	/// Get the number of packages in the index
	size_t size() const { return m_buf ? *(int*)m_buf / sizeof(int) : 0; }

	/**
	 * Get the ID of a package given its name.
	 *
	 * If not found, returns -1.
	 */
	int byName(const std::string& name) const;

	/**
	 * Get a package name given its ID.
	 *
	 * If not found, returns the empty string.
	 */
	std::string byID(int id) const
	{
		if (id >= 0 || static_cast<unsigned>(id) < size())
			return std::string(m_buf + ((int*)m_buf)[id]);
		return std::string();
	}

	/// Get the number of packages in the index
	int size(int id) const
	{
		if (id < 0 || (unsigned)id >= size())
			return 0;
		if ((unsigned)id == size() - 1)
			return m_size - ((int*)m_buf)[id] - 1;
		else
			return ((int*)m_buf)[id + 1] - ((int*)m_buf)[id] - 1;
	}
};

}
}

// vim:set ts=4 sw=4:
#endif
