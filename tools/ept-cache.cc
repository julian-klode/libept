/*
 * ept-cache - Commandline interface to the ept library
 *
 * Copyright (C) 2007--2010  Enrico Zini <enrico@debian.org>
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

#include "Environment.h"
#include "EptCacheOptions.h"
#include "info.h"
#include "filters.h"
#include "utils.h"

#include <ept/apt/apt.h>
#include <ept/apt/packagerecord.h>
#include <ept/debtags/debtags.h>
#include <ept/debtags/vocabulary.h>
#include <ept/popcon/popcon.h>
#include <ept/popcon/local.h>
#include <tagcoll/expression.h>
#include <ept/axi/axi.h>

#include <wibble/regexp.h>
#include <wibble/string.h>

#include <algorithm>
#include <iostream>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * * For packages
 *
 * Filters:
 *  + keyword search
 *  + tag expression
 *  - min/max popularity
 *  - installed status
 *
 * Sort orders
 *  + unsorted
 *  + alphabetical (?)
 *  + by popularity
 *  + by representativeness (only when installed)
 *  - by xapian score
 *
 * Show
 *  + Names only
 *  + Names and short descriptions
 *  + Full fields (dumpavail)
 *  + Nothing (return value à la grep)
 *
 *
 * * For tags
 *
 * Filters
 *
 *  - keyword search
 *  - relevance search
 *
 * Sort orders
 *
 *  - alphabetic order
 *  - relevance order
 *  - xapian order (automatically given by the data source)
 *
 * Show
 *  - Names only
 *  - Names and short descriptions
 *  - Full fields (dumpavail)
 *  - Nothing (return value à la grep)
 */

using namespace std;
using namespace tagcoll;
using namespace wibble;
using namespace ept;
using namespace ept::debtags;
using namespace ept::apt;

static const int DEFAULT_QUALITY_CUTOFF = 50;

// Database of expression macros
struct ExpressionMacros : public map<string, string>
{
	ExpressionMacros() {
		insert(make_pair("gui", "role::program && (interface::x11 || interface::3d)"));
		insert(make_pair("cmdline", "role::program && interface::commandline"));
		insert(make_pair("game", "role::program && game::*"));
		insert(make_pair("devel", "devel::* && !role::shared-lib"));
		insert(make_pair("clean", "!role::shared-lib && !role::app-data"));
	}
} expressionMacros;

// Sorting infrastructure
template<typename SORTER>
struct Negator
{
	const SORTER& s;

	Negator(const SORTER& s) : s(s) {}

	bool operator()(const Info* a, const Info* b) const
	{
		return s(b, a);
	}
};
template<typename SORTER>
Negator<SORTER> negator(const SORTER& s)
{
	return Negator<SORTER>(s);
}
bool popularitySort(const Info* a, const Info* b)
{
	return a->popcon * a->xapian_score < b->popcon * b->xapian_score;
}
bool tfidfSort(const Info* a, const Info* b)
{
	return a->tfidf < b->tfidf;
}
bool nameSort(const Info* a, const Info* b)
{
	return a->name < b->name;
}
bool downloadSizeSort(const Info* a, const Info* b)
{
	return a->record->packageSize() < b->record->packageSize();
}
bool installedSizeSort(const Info* a, const Info* b)
{
	return a->record->installedSize() < b->record->installedSize();
}

/// Consumer for package filters
struct Consumer
{
	size_t count;
	int limit;

	Consumer() : count(0), limit(-1) {}
	virtual ~Consumer() {}

	// Set the maximum number of items to consume from now on
	void setLimit(int limit) { this->limit = limit; }

	// Set the maximum number of items to consume from now on to unlimited
	void resetLimit() { limit = -1; }

	// Consume one Info item; returns true if it can consume more, false if
	// production should stop for this consumer
	virtual bool operator()(Info& i)
	{
		++count;
		if (limit == -1)
			return true;
		else if (limit > 0)
		{
			--limit;
			return true;
		} else
			return false;
	}

	// Signal that no more items will be produced
	virtual void done() {}
};

struct Sorter : public Consumer
{
	vector<Info*> data;
	bool reverse;
	char sortType;
	Consumer* out;

	Sorter(const std::string& stype, Consumer* out)
		: reverse(false), sortType(0), out(out)
	{
		// Parse the sort type
		for (string::const_iterator i = stype.begin();
				i != stype.end(); ++i)
			switch (*i)
			{
				case '-': reverse = true; break;
				default: if (!sortType) sortType = *i; break;
			}
		// Early warning to avoid complaining after generation was done
		if (sortType != 'd' && sortType != 'i' && sortType != 'n' && sortType != 'p' && sortType != 't' && sortType != 'x')
			throw wibble::exception::Consistency("parsing sort type", "invalid sort type (use 'list' for a list)");
	}
	virtual ~Sorter()
	{
		if (out) delete out;
		for (vector<Info*>::iterator i = data.begin();
				i != data.end(); ++i)
			delete *i;
	}

	/**
	 * Disconnect the chained consumer so that it will not be deallocated when
	 * the Sorter object is deleted.
	 *
	 * @returns The chained consumer
	 */
	Consumer* disconnectConsumer()
	{
		Consumer* res;
		this->out = 0;
		return res;
	}

	virtual bool operator()(Info& i)
	{
		if (!Consumer::operator()(i))
			return false;
		data.push_back(new Info(i));
		return true;
	}

	virtual void done()
	{
		bool generateReversed = false;

		// Perform sorting
		switch (sortType)
		{
			case 'd':
				// Download size
				for (vector<Info*>::iterator i = data.begin();
						i != data.end(); ++i)
					(*i)->wantRecord();
				if (reverse)
					std::sort(data.begin(), data.end(), negator(downloadSizeSort));
				else
					std::sort(data.begin(), data.end(), downloadSizeSort);
				break;
			case 'i':
				// Installed size
				for (vector<Info*>::iterator i = data.begin();
						i != data.end(); ++i)
					(*i)->wantRecord();
				if (reverse)
					std::sort(data.begin(), data.end(), negator(installedSizeSort));
				else
					std::sort(data.begin(), data.end(), installedSizeSort);
				break;
			case 'n':
				// Alphabetical
				if (reverse)
					std::sort(data.begin(), data.end(), negator(nameSort));
				else
					std::sort(data.begin(), data.end(), nameSort);
				break;
			case 'p':
				// Popularity
				for (vector<Info*>::iterator i = data.begin();
						i != data.end(); ++i)
					(*i)->wantPopcon();
				if (reverse)
					std::sort(data.begin(), data.end(), negator(popularitySort));
				else
					std::sort(data.begin(), data.end(), popularitySort);
				break;
			case 't':
				// TFIDF
				for (vector<Info*>::iterator i = data.begin();
						i != data.end(); ++i)
					(*i)->wantPopconLocal();
				if (reverse)
					//std::sort(data.begin(), data.end(), negator(tfidfSort));
					std::sort(data.begin(), data.end(), negator(tfidfSort));
				else
					std::sort(data.begin(), data.end(), tfidfSort);
				break;
			case 'x':
				// Xapian score

				// We need to flip the other way round here, because xapian
				// produces results in reverse score order
				generateReversed = !reverse;
				break;
			default:
				throw wibble::exception::Consistency("sorting", "invalid sort type (use 'list' for a list)");
		}

		if (generateReversed)
		{
			for (vector<Info*>::reverse_iterator i = data.rbegin();
					i != data.rend(); ++i)
				if (!(*out)(**i))
					break;
		}
		else
		{
			for (vector<Info*>::iterator i = data.begin();
					i != data.end(); ++i)
				if (!(*out)(**i))
					break;
		}
	}
};

struct NullPrinter : public Consumer
{
	virtual ~NullPrinter() {}
};

struct NamePrinter : public Consumer
{
	ostream& out;
	NamePrinter(ostream& out) : out(out) {}

	virtual ~NamePrinter() {}
	virtual bool operator()(Info& i)
	{
		if (!Consumer::operator()(i))
			return false;
		out << i.name << endl;
		return true;
	}
};

struct ShortDescPrinter : public Consumer
{
	ostream& out;
	ShortDescPrinter(ostream& out) : out(out) {}
	virtual ~ShortDescPrinter() {}
	virtual bool operator()(Info& i)
	{
		if (!Consumer::operator()(i))
			return false;
		i.wantRecord();
		out << i.name << " - " <<
			i.record->shortDescription("(short description not available)") << endl;
		return true;
	}
};

struct TagcollPrinter : public Consumer
{
	ostream& out;
	TagcollPrinter(ostream& out) : out(out) {}
	virtual ~TagcollPrinter() {}
	virtual bool operator()(Info& i)
	{
		if (!Consumer::operator()(i))
			return false;
		i.wantTags();
		out << i.name << ": " << i.tags << endl;
		return true;
	}
};

struct FullRecordPrinter : public Consumer
{
	bool hasDebtags;
	bool hasPopcon;
	bool hasPopconLocal;
	ostream& out;
	FullRecordPrinter(ostream& out) : out(out)
	{
		hasDebtags = env().debtags().hasData();
		hasPopcon = env().popcon().hasData();
		hasPopconLocal = env().popconLocal().hasData();
	}
	virtual ~FullRecordPrinter() {}
	virtual bool operator()(Info& info)
	{
		if (!Consumer::operator()(info))
			return false;
		info.wantRecord();
		info.wantTags();
		PackageRecord& record = *info.record;
		bool tagsPrinted = false;
		for (size_t i = 0; i < record.size(); ++i)
		{
			if (record.name(i) == "Tag")
			{
				tagsPrinted = true;
				if (!info.tags.empty())
					out << "Tag: " << info.tags << std::endl;
			} else {
				out << record.field(i);
			}
		}
		if (!tagsPrinted)
			if (!info.tags.empty())
				out << "Tag: " << info.tags << std::endl;
		if (hasPopcon)
		{
			info.wantPopcon();
			out << "Popcon: " << info.popcon << endl;
		}
		if (hasPopconLocal)
		{
			info.wantPopconLocal();
			out << "TFIDF: " << info.tfidf << endl;
		}
		if (info.has_xapian)
		{
			out << "Search-Score: " << info.xapian_score << endl;
		}
		out << std::endl;
		return true;
	}
};

/// Send a tag stream to an Info consumer
class OutputInfo : public wibble::mixin::OutputIterator<OutputInfo>
{
	const filter::Base& filter;
	Consumer& out;
	// When true, the consumer is ignoring the input
	bool done;

public:
	OutputInfo(const filter::Base& filter, Consumer& out)
		: filter(filter), out(out), done(false) {}

	template<typename ITEMS, typename TAGS>
	OutputInfo& operator=(const std::pair<ITEMS, TAGS>& data)
	{
		if (done) return *this;
		for (typename ITEMS::const_iterator i = data.first.begin();
				i != data.first.end(); ++i)
		{
			if (!env().apt().isValid(*i)) continue;
			Info info(*i);
			info.has_tags = true;
			info.tags = data.second;
			if (filter(info))
				if (!out(info))
				{
					done = true;
					break;
				}
		}
		return *this;
	}
};

/**
 * Generate an Info stream and send it to a consumer.
 *
 * The sorting order will depend on what is the data provider that is chosen to
 * initiate the stream, and is thus undefined.
 */
struct Generator
{
	filter::And filters;

	Xapian::QueryParser qp;
	Xapian::Stem stem;

	Generator() : stem("en")
	{
		qp.set_default_op(Xapian::Query::OP_AND);
		qp.set_database(env().axi());
		qp.set_stemmer(stem);
		qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
		qp.add_prefix("pkg", "XP");
		qp.add_boolean_prefix("tag", "XT");
		qp.add_boolean_prefix("sec", "XS");
	}
	
	Xapian::Query makeQuery(const vector<string>& keywords)
	{
		// Add prefixes to tag names
		const Vocabulary& voc = env().voc();
		vector<string> kw;
		for (vector<string>::const_iterator i = keywords.begin();
				i != keywords.end(); ++i)
		{
			if (voc.hasTag(*i))
				kw.push_back("tag:" + *i);
			else
				kw.push_back(*i);
		}
		return qp.parse_query(str::join(kw.begin(), kw.end(), " "),
				Xapian::QueryParser::FLAG_BOOLEAN |
				Xapian::QueryParser::FLAG_LOVEHATE |
				Xapian::QueryParser::FLAG_BOOLEAN_ANY_CASE |
				Xapian::QueryParser::FLAG_WILDCARD |
				Xapian::QueryParser::FLAG_PURE_NOT |
				Xapian::QueryParser::FLAG_SPELLING_CORRECTION);
	}

	// Generate all the packages, without records
	void generateNames(Consumer& out)
	{
		debug("Generate iterating names\n");
		for (Apt::iterator i = env().apt().begin();
				i != env().apt().end(); ++i)
		{
			Info info(*i);
			if (filters(info))
				if (!out(info))
					break;
		}
	}

	// Generate all the packages, with records
	void generateRecords(Consumer& out)
	{
		debug("Generate iterating records\n");
		PackageRecord rec;
		for (Apt::record_iterator i = env().apt().recordBegin();
				i != env().apt().recordEnd(); ++i)
		{
			rec.scan(*i);
			Info info(rec.package());
			info.record = &rec;
			if (filters(info))
				if (!out(info))
					break;
		}
	}

	// Generate after a keyword search
	void keywordXapianSearch(const vector<string>& keywords, Consumer& out, int qualityCutoff = DEFAULT_QUALITY_CUTOFF)
	{
		debug("Generate with xapian\n");
		Xapian::Enquire enquire(env().axi());

		// Set up the base query
		Xapian::Query query = makeQuery(keywords);
		enquire.set_query(query);
		debug("Xapian query: %s\n", query.get_description().c_str());

#if 0
		// Get a set of tags to expand the query
		vector<string> expand = env().textsearch().expand(enquire);

		// Build the expanded query
		Xapian::Query expansion(Xapian::Query::OP_OR, expand.begin(), expand.end());
		enquire.set_query(Xapian::Query(Xapian::Query::OP_OR, query, expansion));
		debug("Expanded Xapian query: %s\n", enquire.get_query().get_description().c_str());
#endif

		//cerr << "Q: " << enquire.get_query().get_description() << endl;
		fromXapianEnquire(enquire, out, qualityCutoff);
	}

	void fromXapianEnquire(Xapian::Enquire& enquire, Consumer& out, int qualityCutoff = DEFAULT_QUALITY_CUTOFF, const Xapian::MatchDecider* md = 0)
	{
		// Retrieve the first result, to compute the cutoff score
		Xapian::MSet first = enquire.get_mset(0, 1, 0, 0, md);
		Xapian::MSetIterator ifirst = first.begin();
		if (ifirst == first.end())
			// If there are no results, quit now
			return;
		// Set a percentage cutoff based on the quality of the first results
		debug("  Reference value for quality cutoff: %d%%\n", ifirst.get_percent());
		Xapian::percent cutoff = ifirst.get_percent() * qualityCutoff / 100;
		debug("  Quality cutoff at: %d%%\n", cutoff);
		enquire.set_cutoff(cutoff);

		bool done = false;
		for (size_t pos = 0; !done; pos += 20)
		{
			// Limit to 1000 matches
			Xapian::MSet matches = enquire.get_mset(pos, 20, 0, 0, md);
			if (matches.size() < 20)
				done = true;
			for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); ++i)
			{
				// Filter out results that apt doesn't know
				if (!env().apt().isValid(i.get_document().get_data()))
				{
					debug("  Not in apt database: %s\n", i.get_document().get_data().c_str());
					continue;
				}
				// Create the info element
				Info info(i.get_document().get_data());
				info.has_xapian = true;
				info.xapian_score = i.get_percent();

				debug("  Producing %s (%d%%)\n", info.name.c_str(), info.xapian_score);

				// Pass it on to the consumer
				if (filters(info))
					if (!out(info))
					{
						done = true;
						break;
					}
			}
		}
	}

	void keywordAptSearch(const vector<string>& keywords, Consumer& out)
	{
		debug("Generate iterating records plus a keyword search\n");
		filters.acquire_front(new filter::Description(keywords));
		generateRecords(out);
	}

	void debtagsSearch(const set<std::string>& wantedTags, Consumer& out)
	{
		debug("Generate by querying debtags\n");
		env().debtags().outputHavingTags(wantedTags, OutputInfo(filters, out));
	}
};


/**
 * Instantiate the consumer defined by the output options.
 *
 * If no output option has been specified, it returns the given default.
 * Otherwise it creates a new default consumer and deletes the default one.
 */
Consumer* createPrinter(const wibble::commandline::EptCacheOptions& opts, Consumer* def)
{
	Consumer* res = 0;
	if (opts.out_names->boolValue())
		res = new NamePrinter(cout);
	else if (opts.out_short->boolValue())
		res = new ShortDescPrinter(cout);
	else if (opts.out_full->boolValue())
		res = new FullRecordPrinter(cout);
	else if (opts.out_quiet->boolValue())
		res = new NullPrinter();
	else if (opts.out_tagcoll->boolValue())
		res = new TagcollPrinter(cout);
	else 
		res = def;
	if (res != def)
		delete def;
	return res;
}

bool usesXapian(wibble::commandline::EptCacheOptions& opts)
{
	return opts.hasNext() && axi::timestamp() > 0;
}

void generate(wibble::commandline::EptCacheOptions& opts, Consumer& output,
				int defaultXapianLimit = -1, int defaultXapianQualityCutoff = DEFAULT_QUALITY_CUTOFF)
{
	Generator gen;

	// Append filters
	if (opts.filter_tagexpr->isSet())
		gen.filters.acquire(new filter::TagExpression(opts.filter_tagexpr->stringValue()));

	if (opts.filter_type->isSet())
	{
		ExpressionMacros::const_iterator i = expressionMacros.find(opts.filter_type->stringValue());
		if (i == expressionMacros.end())
			throw wibble::exception::Consistency("parsing filter type", "invalid filter type (use 'list' for a list)");
		gen.filters.acquire(new filter::TagExpression(i->second));
	}

	// Configure the output consumer with sorters and limits
	Consumer* cons = &output;

	// If we sort, we introduce an intermediate step
	if (opts.out_sort->isSet())
		cons = new Sorter(opts.out_sort->stringValue(), cons);

	// Set the output limit where it's needed
	bool xapian = usesXapian(opts);
	if (xapian)
	{
		if (opts.out_sort->isSet())
		{
			// If we use Xapian and we have a sorter, we need a cutoff
			// at the quality score: since the sorter will scramble the
			// xapian relevance scoring, we only need to feed good
			// results to it

			// Keep the default Xapian quality cutoff set by the user, then

			// Set the match limit on the printer side
			if (opts.out_limit->isSet())
				output.setLimit(opts.out_limit->intValue());
		} else {
			// If we use Xapian and don't have a sorter, we can keep
			// generating less relevant results as the xapian sorting
			// order will help the user to make sense of the results
			defaultXapianQualityCutoff = -1;

			// Set the match limit at the beginning of the output chain
			if (opts.out_limit->isSet())
				cons->setLimit(opts.out_limit->intValue());
			else if (defaultXapianLimit != -1)
				// Xapian shows the best matches first, so we can greatly
				// reduce the limit
				cons->setLimit(defaultXapianLimit);
		}
	}
	else
	{
		// If we don't use Xapian, the packages arrive in undefined
		// order, so we need to feed all of them to the sorter
		// because we don't know when the top scored packages will
		// be generated, or to the output

		// Set the match limit on the printer side
		if (opts.out_limit->isSet())
			output.setLimit(opts.out_limit->intValue());
	}

	if (opts.out_cutoff->isSet())
		defaultXapianQualityCutoff = opts.out_cutoff->intValue();

	if (opts.hasNext())
	{
		// We have keywords
		vector<string> keywords;
		while (opts.hasNext())
			keywords.push_back(toLower(opts.next()));
		if (axi::timestamp() > 0)
			gen.keywordXapianSearch(keywords, *cons, defaultXapianQualityCutoff);
		else
			gen.keywordAptSearch(keywords, *cons);
	} else {
		// No keyword search
		if (opts.out_names->boolValue())
			gen.generateNames(*cons);
		else
			gen.generateRecords(*cons);
	}

	// Signal the end of input to the consumers
	cons->done();
}

struct BlacklistDecider : public Xapian::MatchDecider
{
	std::set<std::string> blacklist;
	BlacklistDecider() {}
	BlacklistDecider(const std::set<std::string>& blacklist) : blacklist(blacklist) {}

	virtual bool operator()(const Xapian::Document& doc) const
	{
		return blacklist.find(doc.get_data()) == blacklist.end();
	}
};

Xapian::Query makeRelatedQuery(const std::string& pkgname)
{
        Xapian::Enquire enquire(env().axi());
        
        // Retrieve the document for the given package
        enquire.set_query(Xapian::Query("XP"+pkgname));
        Xapian::MSet matches = enquire.get_mset(0, 1);
        Xapian::MSetIterator mi = matches.begin();
        if (mi == matches.end()) return Xapian::Query();
        Xapian::Document doc = mi.get_document();
 
        // Return the query to get the list of similar documents
        return Xapian::Query(Xapian::Query::OP_OR, doc.termlist_begin(), doc.termlist_end());
}

void generateRelated(wibble::commandline::EptCacheOptions& opts, Consumer& output,
				int defaultXapianLimit = -1, int defaultXapianQualityCutoff = DEFAULT_QUALITY_CUTOFF)
{
	Generator gen;

	// Append filters
	if (opts.filter_tagexpr->isSet())
		gen.filters.acquire(new filter::TagExpression(opts.filter_tagexpr->stringValue()));

	if (opts.filter_type->isSet())
	{
		ExpressionMacros::const_iterator i = expressionMacros.find(opts.filter_type->stringValue());
		if (i == expressionMacros.end())
			throw wibble::exception::Consistency("parsing filter type", "invalid filter type (use 'list' for a list)");
		gen.filters.acquire(new filter::TagExpression(i->second));
	}

	// Configure the output consumer with sorters and limits
	Consumer* cons = &output;

	// If we sort, we introduce an intermediate step
	if (opts.out_sort->isSet())
		cons = new Sorter(opts.out_sort->stringValue(), cons);

	// Set the output limit where it's needed
	if (opts.out_sort->isSet())
	{
		// If we use Xapian and we have a sorter, we need a cutoff
		// at the quality score: since the sorter will scramble the
		// xapian relevance scoring, we only need to feed good
		// results to it

		// Keep the default Xapian quality cutoff set by the user, then

		// Set the match limit on the printer side
		if (opts.out_limit->isSet())
			output.setLimit(opts.out_limit->intValue());
	} else {
		// If we use Xapian and don't have a sorter, we can keep
		// generating less relevant results as the xapian sorting
		// order will help the user to make sense of the results
		defaultXapianQualityCutoff = -1;

		// Set the match limit at the beginning of the output chain
		if (opts.out_limit->isSet())
			cons->setLimit(opts.out_limit->intValue());
		else if (defaultXapianLimit != -1)
			// Xapian shows the best matches first, so we can greatly
			// reduce the limit
			cons->setLimit(defaultXapianLimit);
	}

	if (opts.out_cutoff->isSet())
		defaultXapianQualityCutoff = opts.out_cutoff->intValue();

	Xapian::Enquire enq(env().axi());
	string name = opts.next();
	Xapian::Query query = makeRelatedQuery(name);
	BlacklistDecider blacklister;
	debug("Excluding '%s' from the results\n", name.c_str());
	blacklister.blacklist.insert(name);
	// We can even easily compute packages related to a list of packages
	while (opts.hasNext())
	{
		name = opts.next();
		debug("Excluding '%s' from the results\n", name.c_str());
		blacklister.blacklist.insert(name);
		if (!env().apt().isValid(name))
			throw wibble::exception::Consistency("reading package names", "package "+name+" does not exist");
		query = Xapian::Query(Xapian::Query::OP_AND, query, makeRelatedQuery(name));
	}
	//enq.register_match_decider("blacklist", &blacklister);
	//gen.filters.acquire(new filter::Blacklist(seen));
	enq.set_query(query);

	// Generate from the Xapian enquire
	//cerr << "Q: " << enq.get_query().get_description() << endl;
	gen.fromXapianEnquire(enq, *cons, defaultXapianQualityCutoff, &blacklister);

	// Signal the end of input to the consumers
	cons->done();
}

bool printOutOptionsHelpIfNeeded(wibble::commandline::EptCacheOptions& opts)
{
	if (opts.filter_type->isSet() && opts.filter_type->stringValue() == "list")
	{
		for (ExpressionMacros::const_iterator i = expressionMacros.begin();
				i != expressionMacros.end(); ++i)
			cout << i->first << ": " << i->second << endl;
		return true;
	}

	if (opts.out_sort->isSet() && opts.out_sort->stringValue() == "list")
	{
		cout << "Available sort options:" << endl
			 << endl
			 << "download size - Sort by download size" << endl
			 << "installed size - Sort by installed size" << endl
			 << "name - Sort alphabetically by package name" << endl
			 << "popularity - Sort by popcon popularity" << endl
			 << "tfidf - Sort by how much a package makes a system unique" << endl
			 << "xapian - Sort by xapian relevance score" << endl
			 << endl
			 << "It is sufficient to provide a nonambiguous prefix of the sort type." << endl
			 << "Add a '-' to the begining or end to reverse the sort order." << endl
			 << "Example:" << endl
			 << "  ept-cache search -s p- image editor" << endl;
		return true;
	}
	return false;
}

int success_if_had_output(Consumer* output)
{
	int count = output->count;
	delete output;
	return count > 0 ? 0 : 1;
}

int main(int argc, const char* argv[])
{
	wibble::commandline::EptCacheOptions opts;
	bool warn_non_root_on_error = false;

	try {
		// Install the handler for unexpected exceptions
		wibble::exception::InstallUnexpected installUnexpected;

		if (opts.parse(argc, argv))
			return 0;

		if (opts.out_verbose->boolValue())
			::Environment::get().verbose(true);

		if (opts.out_debug->boolValue())
			::Environment::get().debug(true);

		// show <pkg>
		// Similar to apt-cache show <pkg>, but add extra metadata to the output."
		if (opts.foundCommand() == opts.show)
		{
			env().init();
			Consumer* output = createPrinter(opts, new FullRecordPrinter(cout));
			while (opts.hasNext())
			{
				string name = opts.next();

				if (env().apt().isValid(name))
				{
					Info i(name);
					(*output)(i);
				} else {
					verbose("Package %s not found", name.c_str());
				}
			}
			return success_if_had_output(output);
		}
		else if (opts.foundCommand() == opts.search)
		{
			if (printOutOptionsHelpIfNeeded(opts))
				return 1;
			env().init();
			Consumer* output = createPrinter(opts, new ShortDescPrinter(cout));

			// Generate the results
			generate(opts, *output, 50, DEFAULT_QUALITY_CUTOFF);

			return success_if_had_output(output);
		}
		// dumpavail
		// Output the full package database, with all the extra info available
		else if (opts.foundCommand() == opts.dumpavail)
		{
			if (printOutOptionsHelpIfNeeded(opts))
				return 1;
			env().init();
			Consumer* output = createPrinter(opts, new FullRecordPrinter(cout));

			// Generate the results
			generate(opts, *output, -1, -1);

			return success_if_had_output(output);
		}
		// related
		// List packages related to the given one
		else if (opts.foundCommand() == opts.related)
		{
			if (printOutOptionsHelpIfNeeded(opts))
				return 1;
			env().init();
			Consumer* output = createPrinter(opts, new ShortDescPrinter(cout));

			// Generate the results
			generateRelated(opts, *output, 15, DEFAULT_QUALITY_CUTOFF);

			return success_if_had_output(output);
		}
		// reindex
		// Rebuilds indexes (requires root)
		else if (opts.foundCommand() == opts.reindex)
		{
			warn_non_root_on_error = true;

			mode_t prev_umask = umask(022);

			int exitcode;
			if (opts.out_quiet->boolValue())
				exitcode = system("update-apt-xapian-index --quiet");
			else
				exitcode = system("update-apt-xapian-index");
			if (exitcode != 0)
			{
				stringstream str;
				str << "got an exit code of " << exitcode;
				throw wibble::exception::Consistency("running update-apt-xapian-index", str.str());
			}
				
			// TODO: if verbose, print the various data files used

			umask(prev_umask);
		}
		// info
		// Show information about the data providers
		else if (opts.foundCommand() == opts.info)
		{
			env().init();
			if (env().debtags().hasData())
				cout << "Debtags: enabled." << endl;
			else
				cout << "Debtags: disabled.  To enable it, run 'debtags update' as root." << endl;

			if (env().popcon().hasData())
				cout << "Popcon: enabled." << endl;
			else
				cout << "Popcon: disabled.  To enable it, download http://popcon.debian.org/all-popcon-results.txt.gz" << endl
					 << "        in /var/lib/popcon/ and run 'ept-cache reindex' as root" << endl;

			if (env().popconLocal().hasData())
				cout << "Popcon local scan: enabled." << endl;
			else
				cout << "Popcon local scan: disabled.  To enable it, install the popularity-contest package and" << endl
					 << "        enable it to run." << endl;

			time_t xts = axi::timestamp();
			if (xts > 0)
				if (xts < env().apt().timestamp())
					cout << "Xapian: enabled but not up to date.  To update it, run 'update-apt-xapian-index' as root." << endl;
				else
					cout << "Xapian: enabled and up to date." << endl;
			else
				cout << "Xapian: disabled.  To enable it, run 'ept-cache reindex' as root" << endl;

			// TODO: if verbose, print the various data files used
		}
		else
			throw wibble::exception::BadOption(string("unhandled command ") +
						(opts.foundCommand() ? opts.foundCommand()->name() : "(null)"));

		return 0;
	} catch (wibble::exception::BadOption& e) {
		cerr << e.desc() << endl;
		opts.outputHelp(cerr);
		return 1;
	} catch (std::exception& e) {
		cerr << e.what() << endl;
		if (warn_non_root_on_error && getuid() != 0)
			cerr << "You may need to be root to perform this operation." << endl;
		return 1;
	} catch (Xapian::DatabaseVersionError& e) {
		cerr << "Xapian " << e.get_type() << ": " << e.get_msg();
		if (!e.get_context().empty())
			cerr << ". Context: " << e.get_context();
		cerr << endl;
		cerr << endl;
		cerr << "Please recreate the database by removing /var/lib/apt-xapian and running ept-cache reindex as root." << endl;
	} catch (Xapian::Error& e) {
		cerr << "Xapian " << e.get_type() << ": " << e.get_msg();
		if (!e.get_context().empty())
			cerr << ". Context: " << e.get_context();
		cerr << endl;
		if (warn_non_root_on_error && getuid() != 0)
			cerr << "You may need to be root to perform this operation." << endl;
		return 1;
	}
}

#include <ept/debtags/debtags.tcc>

// vim:set ts=4 sw=4:
