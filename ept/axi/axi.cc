
/** @file
 * @author Enrico Zini <enrico@enricozini.org>
 * Fast full-text search
 */

/*
 * Copyright (C) 2007  Enrico Zini <enrico@debian.org>
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

#include <ept/config.h>
#include "axi.h"
#include "ept/utils/sys.h"
#include "ept/utils/string.h"
#include <memory>

using namespace std;

namespace ept {
namespace axi {

static std::string m_index_dir = TEXTSEARCH_DB_DIR;

std::string path_dir()
{
	return m_index_dir;
}

std::string path_db()
{
	return str::joinpath(m_index_dir, "/index");
}

time_t timestamp()
{
    string tsfile = str::joinpath(m_index_dir, "update-timestamp");
    return sys::timestamp(tsfile, 0);
}


OverrideIndexDir::OverrideIndexDir(const std::string& path) : old(m_index_dir)
{
	m_index_dir = path;
}

OverrideIndexDir::~OverrideIndexDir()
{
	m_index_dir = old;
}

}
}

// vim:set ts=4 sw=4:
