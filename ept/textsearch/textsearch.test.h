// -*- mode: c++; tab-width: 4; indent-tabs-mode: t -*-
/*
 * popcon test
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

#include <ept/test.h>
#include <ept/textsearch/textsearch.h>
#include <ept/textsearch/maint/path.h>
#include <ept/apt/apt.h>
#include <wibble/sys/fs.h>
#include <set>

namespace ept {
namespace textsearch {
extern size_t max_index;
}
}

using namespace std;
using namespace ept;
using namespace ept::textsearch;
using namespace ept::apt;

struct DirMaker
{
	DirMaker(const std::string& name)
	{
		wibble::sys::fs::mkdirIfMissing(name, 0755);
	}
};

struct TestTextsearch : AptTestEnvironment
{
	DirMaker md;
	Path::OverrideIndexDir oid;
	Apt apt;
	TextSearch textsearch;

	TestTextsearch()
		: md( TEST_ENV_DIR "xapian"), oid( TEST_ENV_DIR "xapian")
	{
		try {
			ept::textsearch::max_index = 1000;
			textsearch.rebuildIfNeeded(apt);
		} catch (Xapian::Error& e) {
			cerr << e.get_type() << " " << e.get_msg() << " " << e.get_context() << endl;
			throw;
		}
	}

// Access an empty index
	Test empty()
	{
		Path::OverrideIndexDir oid("./empty");
		TextSearch empty;
		assert_eq(empty.timestamp(), 0);
		assert(!empty.hasData());
		assert(empty.needsRebuild(apt));
		/*
		  Xapian::Enquire enq(empty.db());
		  empty.search(enq, "apt");
		  Xapian::MSet matches = enq.get_mset(0, 100);
		  assert_eq(matches.size(), 0u);
		*/
	}

// Very basic access
	Test basicAccess()
	{
		assert(textsearch.hasData());
		assert(textsearch.timestamp() > 0);
		assert(!textsearch.needsRebuild(apt));

		Xapian::Enquire enq(textsearch.db());
		enq.set_query(textsearch.makeORQuery("sgml"));
		Xapian::MSet matches = enq.get_mset(0, 100);
		assert(matches.size() > 0);

		// See if the apt package is among the results
		set<string> results;
		for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); ++i)
			results.insert(i.get_document().get_data());
		assert(results.find("sp") != results.end());
	}

// Alternate access using intermediate Xapian::Query objects
	Test queryAccess()
	{
		Xapian::Enquire enq(textsearch.db());
		enq.set_query(textsearch.makeORQuery("sgml"));
		Xapian::MSet matches = enq.get_mset(0, 100);
		assert(matches.size() > 0);

		// See if the apt package is among the results
		set<string> results;
		for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); ++i)
			results.insert(i.get_document().get_data());
		assert(results.find("sp") != results.end());
	}

// Try makePartialORQuery
	Test partialOrQuery()
	{
		Xapian::Enquire enq(textsearch.db());
		enq.set_query(textsearch.makePartialORQuery("sgml"));
		Xapian::MSet matches = enq.get_mset(0, 100);
		assert(matches.size() > 0);

		// See if the apt package is among the results
		set<string> results;
		for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); ++i)
			results.insert(i.get_document().get_data());
		assert(results.find("sp") != results.end());
	}

// Try docidByName
	Test docidByName()
	{
		assert(textsearch.docidByName("sp") != 0);
		assert_eq(textsearch.docidByName("thereisnopackagewiththisname"), 0u);
	}

// Access values
	Test values()
	{
		assert(textsearch.hasData());
		assert(textsearch.timestamp() > 0);
		assert(!textsearch.needsRebuild(apt));

		double dval;
		dval = textsearch.getDoubleValue("autoconf", VAL_APT_INSTALLED_SIZE);
		assert(dval == 2408);
		dval = textsearch.getDoubleValue("autoconf", VAL_APT_PACKAGE_SIZE);
		assert(dval == 741486);
		assert_eq(textsearch.getDoubleValue("thereisnopackagewiththisname", VAL_APT_INSTALLED_SIZE), 0.0);
		assert_eq(textsearch.getDoubleValue("thereisnopackagewiththisname", VAL_APT_PACKAGE_SIZE), 0.0);

		int val;
		val = textsearch.getIntValue("autoconf", VAL_APT_INSTALLED_SIZE);
		assert(val == 2408);
		val = textsearch.getIntValue("autoconf", VAL_APT_PACKAGE_SIZE);
		assert(val == 741486);
		cout << val;
		assert_eq(textsearch.getIntValue("thereisnopackagewiththisname", VAL_APT_INSTALLED_SIZE), 0);
		assert_eq(textsearch.getIntValue("thereisnopackagewiththisname", VAL_APT_PACKAGE_SIZE), 0);
	}

};

// vim:set ts=4 sw=4:
