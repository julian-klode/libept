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

#include "info.h"

#include "Environment.h"

#include <ept/apt/apt.h>
#include <ept/apt/packagerecord.h>
#include <ept/popcon/popcon.h>
#include <ept/popcon/local.h>

using namespace std;
using namespace ept;
using namespace ept::debtags;
using namespace ept::apt;

Info::Info(const std::string& name)
		: name(name), has_xapian(false), xapian_score(100), deallocate_record(false), record(0), has_tags(false), has_popcon(false), has_local(false) {}

Info::~Info()
{
	if (deallocate_record && record)
		delete record;
}

Info::Info(const Info& i)
	: name(i.name), has_xapian(i.has_xapian), xapian_score(i.xapian_score),
	  has_tags(i.has_tags), tags(i.tags), has_popcon(i.has_popcon),
	  popcon(i.popcon), has_local(i.has_local), tfidf(i.tfidf)
{
	if (i.record)
	{
		deallocate_record = true;
		record = new PackageRecord(*i.record);
	} else {
		deallocate_record = false;
		record = 0;
	}
}

Info::Info& Info::operator=(const Info& i)
{
	name = i.name;
	has_xapian = i.has_xapian;
	xapian_score = i.xapian_score;
	has_tags = i.has_tags;
	tags = i.tags;
	has_popcon = i.has_popcon;
	popcon = i.popcon;
	has_local = i.has_local;
	tfidf = i.tfidf;

	if (i.record)
	{
		PackageRecord* newrec = new PackageRecord(*i.record);
		if (record && deallocate_record)
			delete record;
		deallocate_record = true;
		record = newrec;
	} else {
		if (record && deallocate_record)
			delete record;
		deallocate_record = false;
		record = 0;
	}
	return *this;
}

void Info::wantRecord()
{
	if (record) return;
	record = new PackageRecord(env().apt().rawRecord(name));
	deallocate_record = true;
}

void Info::wantTags()
{
	if (has_tags) return;
	tags = env().debtags().getTagsOfItem(name);
	if (tags.empty() && !env().debtags().hasData())
	{
		wantRecord();
		set<string> stags = record->tag();
		for (set<string>::const_iterator i = stags.begin();
				i != stags.end(); ++i)
			tags.insert(env().voc().tagByName(*i));
	}
	has_tags = true;
}

void Info::wantPopcon()
{
	if (has_popcon) return;
	popcon = env().popcon().score(name);
	has_popcon = true;
}

void Info::wantPopconLocal()
{
	if (has_local) return;
	tfidf = env().popconLocal().tfidf(env().popcon(), name);
	has_local = true;
}

#include <ept/debtags/debtags.tcc>

// vim:set ts=4 sw=4:
