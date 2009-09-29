
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

#include <ept/textsearch/textsearch.h>
#include <ept/textsearch/maint/path.h>
#include <ept/apt/apt.h>
#include <ept/apt/packagerecord.h>
//#include <ept/debtags/debtags.h>

#include <wibble/regexp.h>
#include <cctype>
#include <cmath>

#include <xapian/queryparser.h>

#include <algorithm>

#include <iostream>

using namespace std;
using namespace ept::apt;
using namespace ept::debtags;

namespace ept {
namespace textsearch {

size_t max_index = 0;

TextSearch::TextSearch()
	: m_timestamp(0), m_stem("en")
{
	m_timestamp = Path::indexTimestamp();
	if (m_timestamp)
		m_db.add_database(Xapian::Database(Path::index()));
}

std::string TextSearch::toLower(const std::string& str)
{
	std::string res;
	res.reserve(str.size());
	for (std::string::const_iterator i = str.begin(); i != str.end(); ++i)
		res += tolower(*i);
	return res;
}

bool TextSearch::needsRebuild(apt::Apt& apt)
{
	return apt.timestamp() > m_timestamp;
}

void TextSearch::normalize_and_add(Xapian::Document& doc, const std::string& term, int& pos) const
{
	string t = TextSearch::toLower(term);
	string s = m_stem(t);
	doc.add_term(t);
	if (s != t)
		doc.add_term(s);
}

bool TextSearch::rebuildIfNeeded(apt::Apt& apt, const std::vector<const TextSearch::ExtraIndexer*>& extraIndexers)
{
	// Check if a rebuild is needed, and keep a copy of the APT timestamp for
	// saving later
	time_t aptts = apt.timestamp();
	if (aptts <= m_timestamp)
		return false;

	// Reindex
	Xapian::WritableDatabase database(Xapian::Flint::open(Path::index(), Xapian::DB_CREATE_OR_OPEN));
	Xapian::TermGenerator termgen;
	termgen.set_stemmer(m_stem);
	//database.begin_transaction();
	PackageRecord rec;
	size_t count = 0;
	for (Apt::record_iterator i = apt.recordBegin();
			i != apt.recordEnd(); ++i, ++count)
	{
		// If we are testing, we can set a limit to how many packages we index,
		// to avoid it taking too much time
		if (max_index != 0 && count > max_index)
			break;

		rec.scan(*i);

		Xapian::Document doc;
		doc.set_data(rec.package());

		string pkgid = "XP" + rec.package();
		//std::cerr << "Add " << pkgid << ": " << idx << std::endl;
		doc.add_term(pkgid);

		// Index tags as well
		set<string> tags = rec.tag();
		for (set<string>::const_iterator ti = tags.begin();
				ti != tags.end(); ++ti)
			doc.add_term("XT"+*ti);

		termgen.set_document(doc);
		termgen.index_text_without_positions(rec.package());
		termgen.index_text_without_positions(rec.description());

		// Add the values
		doc.add_value(VAL_APT_INSTALLED_SIZE, Xapian::sortable_serialise(rec.installedSize()));
		doc.add_value(VAL_APT_PACKAGE_SIZE, Xapian::sortable_serialise(rec.packageSize()));

		if (m_timestamp)
			database.replace_document(pkgid, doc);
		else
			database.add_document(doc);
	}

	//database.commit_transaction();

	if (!m_timestamp)
		m_db.add_database(Xapian::Database(Path::index()));
	else
		m_db.reopen();

	m_timestamp = aptts;

	Path::setTimestamp(aptts);

	return true;
}

Xapian::Query TextSearch::makeORQuery(const std::string& keywords) const
{
	wibble::Tokenizer tok(keywords, "[A-Za-z0-9_-]+", REG_EXTENDED);
	return makeORQuery(tok.begin(), tok.end());
}

Xapian::Query TextSearch::makePartialORQuery(const std::string& keywords) const
{
	wibble::Tokenizer tok(keywords, "[A-Za-z0-9_-]+", REG_EXTENDED);
	vector<string> tokens;
	// FIXME: make the Tokenizer iterators properly iterable
	for (wibble::Tokenizer::const_iterator i = tok.begin();
			i != tok.end(); ++i)
		tokens.push_back(*i);
	// Add all the terms starting with 'last'
	if (!tokens.empty())
	{
		string& last = *tokens.rbegin();
		if (last.size() == 1)
			// Ignore one-letter partial terms: they make the query uselessly
			// large and slow, and it's worth just to wait for more characters
			// to come
			tokens.resize(tokens.size() - 1);
		else
			copy(m_db.allterms_begin(last), m_db.allterms_end(last), back_inserter(tokens));
		/*
		for (Xapian::TermIterator t = m_db.allterms_begin(last);
				t != m_db.allterms_end(last); ++t)
			tokens.push_back(*t);
		*/
	}
	return makeORQuery(tokens.begin(), tokens.end());
}

Xapian::docid TextSearch::docidByName(const std::string& pkgname) const
{
	Xapian::PostingIterator i = m_db.postlist_begin("XP"+pkgname);
	if (i == m_db.postlist_end("XP"+pkgname))
		return 0;
	else
		return *i;
}

struct TagFilter : public Xapian::ExpandDecider
{
	virtual bool operator()(const std::string &term) const { return term[0] == 'T'; }
};

static TagFilter tagFilter;

vector<string> TextSearch::expand(Xapian::Enquire& enq) const
{
	Xapian::RSet rset;
	// Get the top 5 results as 'good ones' to compute the search expansion
	Xapian::MSet mset = enq.get_mset(0, 5);
	for (Xapian::MSet::iterator i = mset.begin(); i != mset.end(); ++i)
		rset.add_document(i);
	// Get the expanded set, only expanding the query with tag names
	Xapian::ESet eset = enq.get_eset(5, rset, &tagFilter);
	vector<string> res;
	for (Xapian::ESetIterator i = eset.begin(); i != eset.end(); ++i)
		res.push_back(*i);
	return res;
}

Xapian::Query TextSearch::makeRelatedQuery(const std::string& pkgname) const
{
	Xapian::Enquire enquire(db());
	
	// Retrieve the document for the given package
	enquire.set_query(Xapian::Query("XP"+pkgname));
	Xapian::MSet matches = enquire.get_mset(0, 1);
	Xapian::MSetIterator mi = matches.begin();
	if (mi == matches.end()) return Xapian::Query();
	Xapian::Document doc = mi.get_document();

	// Return the query to get the list of similar documents
	return Xapian::Query(Xapian::Query::OP_OR, doc.termlist_begin(), doc.termlist_end());
}

double TextSearch::getDoubleValue(const std::string& pkgname, Xapian::valueno val_id) const
{
	Xapian::docid id = docidByName(pkgname);
	if (id == 0)
		return 0.0;
	Xapian::Document doc = db().get_document(id);
	string val = doc.get_value(val_id);
	if (val.empty())
		return 0.0;
	else
		return Xapian::sortable_unserialise(val);
}

int TextSearch::getIntValue(const std::string& pkgname, Xapian::valueno val_id) const
{
	Xapian::docid id = docidByName(pkgname);
	if (id == 0)
		return 0;
	Xapian::Document doc = db().get_document(id);
	string val = doc.get_value(val_id);
	if (val.empty())
		return 0;
	else
		return (int)nearbyint(Xapian::sortable_unserialise(val));
}

}
}

// vim:set ts=4 sw=4:
