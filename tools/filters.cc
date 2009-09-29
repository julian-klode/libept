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

#include "filters.h"
#include "info.h"
#include "utils.h"

#include <ept/apt/packagerecord.h>

#include <wibble/regexp.h>

using namespace std;

namespace filter {

And::~And()
{
	for (iterator i = begin(); i != end(); ++i)
		delete *i;
}
void And::acquire_front(Base* filter)
{
	insert(begin(), filter);
}
void And::acquire(Base* filter)
{
	push_back(filter);
}
bool And::operator()(Info& info) const
{
	if (empty())
		return true;
	for (const_iterator i = begin(); i != end(); ++i)
		if (!(**i)(info))
			return false;
	return true;
}

Or::~Or()
{
	for (iterator i = begin(); i != end(); ++i)
		delete *i;
}
void Or::acquire_front(Base* filter)
{
	insert(begin(), filter);
}
void Or::acquire(Base* filter)
{
	push_back(filter);
}
bool Or::operator()(Info& info) const
{
	if (empty())
		return true;
	for (const_iterator i = begin(); i != end(); ++i)
		if ((**i)(info))
			return true;
	return false;
}

Description::Description(const std::string& str)
{
	wibble::Tokenizer tok(str, "[^[:blank:]]+", REG_EXTENDED);
	for (wibble::Tokenizer::const_iterator i = tok.begin();
			i != tok.end(); ++i)
		keywords.push_back(toLower(*i));
}

bool Description::operator()(Info& info) const
{
	info.wantRecord();
	string name = toLower(info.name);
	string desc = toLower(info.record->description());
	for (std::vector<std::string>::const_iterator i = keywords.begin();
			i != keywords.end(); ++i)
		if (name.find(*i) == string::npos && desc.find(*i) == string::npos)
			return false;
	return true;
}

bool TagExpression::operator()(Info& info) const
{
	info.wantTags();
	return expr(info.tags);
}

bool Blacklist::operator()(Info& info) const
{
	return blacklist.find(info.name) == blacklist.end();
}

}

// vim:set ts=4 sw=4:
