#ifndef EPT_TEXTSEARCH_TEXTSEARCH_H
#define EPT_TEXTSEARCH_TEXTSEARCH_H

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

#include <xapian.h>
#include <vector>
#include <string>

namespace ept {
namespace apt {
class Apt;
class PackageRecord;
}
namespace debtags {
class Debtags;
}
namespace textsearch {

// Allocate value indexes for known values
const Xapian::valueno VAL_APT_INSTALLED_SIZE      =  1;
const Xapian::valueno VAL_APT_PACKAGE_SIZE        =  2;
const Xapian::valueno VAL_POPCON                  = 10;
const Xapian::valueno VAL_ITERATING_RATING        = 20;
const Xapian::valueno VAL_ITERATING_FUNCTIONALITY = 21;
const Xapian::valueno VAL_ITERATING_USABILITY     = 22;
const Xapian::valueno VAL_ITERATING_SECURITY      = 23;
const Xapian::valueno VAL_ITERATING_PERFORMANCE   = 24;
const Xapian::valueno VAL_ITERATING_QUALITY       = 25;
const Xapian::valueno VAL_ITERATING_SUPPORT       = 26;
const Xapian::valueno VAL_ITERATING_ADOPTION      = 27;
// If you need to index a value and cannot edit this file, feel free to use any
// value starting from 1000000


/*
Fallback on apt scan searches when index is not present

Explicitly decide at instantiation (or at any other time) if a rebuild should
be performed.  Just adding a 'rebuildIfNeeded' method would be enough.

17:14 #xapian < enrico> Hello.  I'm finally in a position of writing a library to maintain
                        a xapian index with Debian package descriptions in a Debian system
17:14 #xapian < enrico> I have a question, though
17:14 #xapian < enrico> The descriptions change regularly as people run 'apt-get update'
17:15 #xapian < enrico> I'd need to have a way to update the description index after
                        apt-get update, without rebuilding it from scratch
17:15 #xapian < enrico> Is there some documentation on how to do that?  I can't exactly
                        tell Xapian "the new description for package foo is this" because
                        I'd need the xapian id
19:11 #xapian < omega> you can add a unique term with a boolean prefix?
19:11 #xapian < omega> like Qpackage-name
19:11 #xapian < omega> then you search for it and replace_document
19:24 #xapian < richardb> Or indeed, you use the "replace_document()" form which takes a
                          unique_id term.
19:25 #xapian < richardb>         Xapian::docid replace_document(const std::string &
                          unique_term,
19:25 #xapian < richardb>                                        const Xapian::Document &
                          document);
19:43 #xapian < enrico> unique term
19:43 #xapian < enrico> nice!
19:44 #xapian < enrico> can I use a non-alpha prefix, like :package-name ?
19:45 #xapian < enrico> or pkg:package-name
19:45 #xapian < enrico> I suppose I can
*/

/**
 * Maintains and accesses a Xapian index of package descriptions.
 *
 * Contrarily to Debtags and Popcon, TextSearch does not attempt to create the
 * index in the home directory if no system index is found and it is not
 * running as root: this is to avoid secretly building large indexes (>50Mb)
 * in the home directory of users.
 *
 * The idea then is to have root keep the index up to date, possibly running a
 * reindexing tool once a day, or after an apt-get update.
 *
 * This works because the full text search index is useful even if it is
 * slightly out of date.
 */
class TextSearch
{
protected:
	time_t m_timestamp;
	Xapian::Database m_db;
	Xapian::Stem m_stem;

	/// Return a lowercased copy of the string
	static std::string toLower(const std::string& str);

	/**
	 * Add normalised tokens computed from the string to the document doc.
	 *
	 * pos is used as a sequence generator for entering the token position in
	 * the document.
	 */
	void normalize_and_add(Xapian::Document& doc, const std::string& term, int& pos) const;

public:
	struct ExtraIndexer
	{
		virtual ~ExtraIndexer() {}
		virtual void operator()(Xapian::Document& doc, const apt::PackageRecord& rec) const = 0;
	};

	TextSearch();

	/// Access the Xapian database
	Xapian::Database& db() { return m_db; }

	/// Access the Xapian database
	const Xapian::Database& db() const { return m_db; }

	/// Timestamp of when the Xapian database was last updated
	time_t timestamp() const { return m_timestamp; }

	/// Returns true if the index has data
	bool hasData() const { return m_timestamp > 0; }

	/// Returns true if the index is older than the Apt database information
	bool needsRebuild(apt::Apt& apt);

	/**
	 * Rebuild the index if needed.
	 *
	 * Allow to specify functors that contribute to the indexing.
	 *
	 * @note This requires write access to the index directory.
	 * @note This is not the main way to update the index: it is provided here
	 *       only as a way to build a draft index for the library tests
	 */
	bool rebuildIfNeeded(
		apt::Apt& apt,
		const std::vector<const ExtraIndexer*>& extraIndexers = std::vector<const ExtraIndexer*>());

	/**
	 * Retrieve a Xapian docid by package name
	 */
	Xapian::docid docidByName(const std::string& pkgname) const;

	/**
	 * Tokenize the string and build an OR query with the resulting keywords
	 */
	Xapian::Query makeORQuery(const std::string& keywords) const;

	/**
	 * Tokenize the string and build an OR query with the resulting keywords.
	 *
	 * The last token in keywords is considered to be typed only partially, to
	 * implement proper search-as-you-type.
	 */
	Xapian::Query makePartialORQuery(const std::string& keywords) const;

	/**
	 * Build a query with the given keywords, specified as iterators of strings
	 */
	template<typename ITER>
	Xapian::Query makeORQuery(const ITER& begin, const ITER& end) const
	{
		std::vector<std::string> terms;
		// Insert both the lowercased and the stemmed lowercased query terms
		for (ITER i = begin; i != end; ++i)
		{
			std::string t = toLower(*i);
			std::string s = m_stem(t);
			terms.push_back(t);
			if (s != t)
				terms.push_back("Z" + s);
		}
		return Xapian::Query(Xapian::Query::OP_OR, terms.begin(), terms.end());
	}

	/// Return a list of tag-based terms that can be used to expand an OR query
	std::vector<std::string> expand(Xapian::Enquire& enq) const;

//	std::vector<std::string> similar(const std::string& pkg);

	/**
	 * Create a query to look for packages similar to the given one
	 */
	Xapian::Query makeRelatedQuery(const std::string& pkgname) const;

	/**
	 * Get the integer value for 
	 */
	double getDoubleValue(const std::string& pkgname, Xapian::valueno val_id) const;

	/**
	 * Get the integer value for 
	 */
	int getIntValue(const std::string& pkgname, Xapian::valueno val_id) const;
};

}
}

// vim:set ts=4 sw=4:
#endif
