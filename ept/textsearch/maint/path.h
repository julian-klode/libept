// -*- mode: c++; indent-tabs-mode: t -*-
/** \file
 * popcon paths
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

#ifndef EPT_TEXTSEARCH_PATH_H
#define EPT_TEXTSEARCH_PATH_H

#include <string>

struct stat;

namespace ept {
namespace textsearch {

/**
 * Singleton class to configure and access the various Popcon paths
 */
class Path
{
public:
	static std::string indexDir();
	static std::string index();

	// Directory where Popcon source data is found
	static void setIndexDir( const std::string &s );

	static int access( const std::string &, int );
	static time_t indexTimestamp();
	static void setTimestamp(time_t ts);

	// RAII-style classes to temporarily override directories
	class OverrideIndexDir
	{
		std::string old;
	public:
		OverrideIndexDir(const std::string& path) : old(Path::indexDir())
		{
			Path::setIndexDir(path);
		}
		~OverrideIndexDir() { Path::setIndexDir(old); }
	};

protected:
	static Path *s_instance;
	static Path &instance();

	// Directory where Popcon source data is found
	std::string m_indexDir;
};

}
}

// vim:set ts=4 sw=4:
#endif
