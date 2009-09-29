// -*- mode: c++; indent-tabs-mode: t -*-

/** \file
 * popcon paths
 */

/*
 * Copyright (C) 2005,2006,2007  Enrico Zini <enrico@debian.org>, Peter Rockai <me@mornfall.net>
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
#include <ept/textsearch/maint/path.h>

#include <wibble/exception.h>
#include <wibble/sys/fs.h>
#include <wibble/string.h>

#include <cstdio>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace wibble;

namespace ept {
namespace textsearch {

Path &Path::instance() {
	if (!s_instance) {
		s_instance = new Path;
		instance().m_indexDir = TEXTSEARCH_DB_DIR;
	}
	return *s_instance;
}

int Path::access( const std::string &s, int m )
{
	return ::access( s.c_str(), m );
}

time_t Path::indexTimestamp()
{
	string tsfile = str::joinpath(instance().indexDir(), "update-timestamp");
	std::auto_ptr<struct stat> st = wibble::sys::fs::stat(tsfile);
	if (st.get())
		return st->st_mtime;
	else
		return 0;
}

void Path::setTimestamp(time_t ts)
{
	string tsfile = str::joinpath(instance().indexDir(), "/update-timestamp");
	FILE* out = fopen(tsfile.c_str(), "wt");
	if (!out)
		throw wibble::exception::File(tsfile, "opening file for truncate/writing");
	if (fprintf(out, "%ld\n", ts) < 0)
		throw wibble::exception::File(tsfile, "writing the modification time");
	if (fclose(out) < 0)
		throw wibble::exception::File(tsfile, "closing the file");
}

void Path::setIndexDir( const std::string &s )
{
	instance().m_indexDir = s;
}

std::string Path::indexDir() { return instance().m_indexDir; }
std::string Path::index() { return str::joinpath(instance().m_indexDir, "/index"); }

Path *Path::s_instance = 0;

}
}

// vim:set ts=4 sw=4:
