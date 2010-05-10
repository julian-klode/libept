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

#ifndef EPTCACHE_FILTERS_H
#define EPTCACHE_FILTERS_H

#include <vector>
#include <string>

#include <tagcoll/expression.h>

/*
 * * For packages
 *
 * Filters:
 *  - keyword search
 *  - tag expression
 *  - min/max popularity
 *  - installed status
 *
 */

class Info;

namespace filter {

/// Interface for package filters
struct Base
{
	virtual ~Base() {}
	virtual bool operator()(Info& i) const = 0;
};

/// ANDs a list of filters, with short-circuit semantics
struct And : public std::vector<Base*>, public Base
{
	virtual ~And();
	// Add a filter at the beginning of the list
	void acquire_front(Base* filter);
	// Add a filter at the end of the list
	void acquire(Base* filter);
	// Match the info if all filters match
	virtual bool operator()(Info& info) const;
};

/// ORs a list of filters, with short-circuit semantics
struct Or : public std::vector<Base*>, public Base
{
	virtual ~Or();
	// Add a filter at the beginning of the list
	void acquire_front(Base* filter);
	// Add a filter at the end of the list
	void acquire(Base* filter);
	// Match the info if one of the filters match
	virtual bool operator()(Info& info) const;
};

/// Negates a filter
struct Not : public Base
{
	Base* filter;
	Not(Base* filter) : filter(filter) {}
	virtual ~Not() { if (filter) delete filter; }
	virtual bool operator()(Info& info) const
	{
		return ! (*filter)(info);
	}
};

/// Text search on package name and description
struct Description : public Base
{
	std::vector<std::string> keywords;

	// Split up the string in keywords and lowercase them
	Description(const std::string& str);

	// Assume split up, lowercased keywords
	Description(const std::vector<std::string>& keywords) : keywords(keywords) {}

	virtual bool operator()(Info& info) const;
};

// Debtags expression filter
struct TagExpression : public Base
{
	tagcoll::Expression expr;

	TagExpression(const std::string& expr) : expr(expr) {}
	TagExpression(const tagcoll::Expression& expr) : expr(expr) {}

	virtual bool operator()(Info& info) const;
};

// Blacklist filter
struct Blacklist : public Base
{
	std::set<std::string> blacklist;

	Blacklist(const std::set<std::string>& blacklist) : blacklist(blacklist) {}

	virtual bool operator()(Info& info) const;
};

}

#endif
// vim:set ts=4 sw=4:
