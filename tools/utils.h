/*
 * ept-cache - Commandline interface to the ept library
 *
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

#ifndef EPTCACHE_UTILS_H
#define EPTCACHE_UTILS_H

#include <string>
#include <ostream>

#include <wibble/singleton.h>
#include <wibble/empty.h>

namespace std {

// Facilities for outputting a tag set
template<typename TAG, typename _Traits>
basic_ostream<char, _Traits>& operator<<(basic_ostream<char, _Traits>& out, const std::set<TAG>& tags)
{
	for (typename std::set<TAG>::const_iterator i = tags.begin();
			i != tags.end(); i++)
		if (i == tags.begin())
			out << i->fullname();
		else
			out << ", " << i->fullname();
	return out;
}

// Facilities for outputting a wibble::Singleton
template<typename TAG, typename _Traits>
basic_ostream<char, _Traits>& operator<<(basic_ostream<char, _Traits>& out, const wibble::Singleton<TAG>& tags)
{
	out << *tags.begin();
	return out;
}

// Facilities for outputting a wibble::Empty
template<typename TAG, typename _Traits>
basic_ostream<char, _Traits>& operator<<(basic_ostream<char, _Traits>& out, const wibble::Empty<TAG>&)
{
	return out;
}

}

// Convert a string to lower case
static inline std::string toLower(const std::string& s)
{
	std::string res;
	for (std::string::const_iterator i = s.begin(); i != s.end(); ++i)
		res += tolower(*i);
	return res;
}

#endif
// vim:set ts=4 sw=4:
