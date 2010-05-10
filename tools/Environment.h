#ifndef EPTCACHE_ENVIRONMENT_H
#define EPTCACHE_ENVIRONMENT_H

/*
 * Common environment for many program parts
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

#include <string>

namespace Xapian {
class Database;
}

namespace ept {
namespace apt {
class Apt;
}
namespace debtags {
class Debtags;
class Vocabulary;
}
namespace popcon {
class Popcon;
class Local;
}
}

class Environment
{
protected:
	/// Apt data provider
	ept::apt::Apt* m_apt;

	/// Debtags data provider
	ept::debtags::Debtags* m_debtags;

	/// Debtags vocabulary data provider
	ept::debtags::Vocabulary* m_vocabulary;

	/// Popcon data provider
	ept::popcon::Popcon* m_popcon;

	/// Popcon local vote data provider
	ept::popcon::Local* m_popconlocal;

	/// TextSearch data provider
	Xapian::Database* m_axi;

	// True when operations should be verbose
	bool _verbose;

	// True when operations should be very verbose
	bool _debug;

	Environment() throw ();
		
public:
	static Environment& get() throw ();

	/**
	 * Initialise the data providers.
	 *
	 * This method must be called before they can be accessed.
	 */
	void init(bool editable = false);

	/// Access the apt data provider
	ept::apt::Apt& apt() { return *m_apt; }

	/// Access the debtags data provider
	ept::debtags::Debtags& debtags() { return *m_debtags; }

	/// Access the tag vocabulary
	ept::debtags::Vocabulary& voc() { return *m_vocabulary; }

	/// Access the popcon data
	ept::popcon::Popcon& popcon() { return *m_popcon; }

	/// Access the popcon data
	ept::popcon::Local& popconLocal() { return *m_popconlocal; }

	/// Access the textsearch data
	Xapian::Database& axi() { return *m_axi; }

	// Accessor methods

	bool verbose() const throw () { return _verbose; }
	bool verbose(bool verbose) throw () { return _verbose = verbose; }

	bool debug() const throw () { return _debug; }
	bool debug(bool debug) throw ()
	{
		// Debug implies verbose
		if (debug)
			_verbose = true;
		return _debug = debug;
	}
};

// Commodity output functions

#ifndef ATTR_PRINTF
 #ifdef GCC
  #define ATTR_PRINTF(string, first) __attribute__((format (printf, string, first)))
 #else
  #define ATTR_PRINTF(string, first)
 #endif
#endif

void fatal_error(const char* fmt, ...) throw() ATTR_PRINTF(1, 2);
void error(const char* fmt, ...) throw() ATTR_PRINTF(1, 2);
void warning(const char* fmt, ...) throw() ATTR_PRINTF(1, 2);
void verbose(const char* fmt, ...) throw() ATTR_PRINTF(1, 2);
void debug(const char* fmt, ...) throw() ATTR_PRINTF(1, 2);
void feedback(const char* fmt, ...) throw() ATTR_PRINTF(1, 2);

static inline Environment& env() { return Environment::get(); }

// vim:set ts=4 sw=4:
#endif
