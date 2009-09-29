#ifndef EPTCACHE_OPTIONS_H
#define EPTCACHE_OPTIONS_H

/*
 * Commandline parser for tagcoll
 *
 * Copyright (C) 2003,2004,2005,2006  Enrico Zini
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
#include <wibble/commandline/parser.h>

namespace wibble {
namespace commandline {

struct EptCacheOptions : public StandardParserWithMandatoryCommand
{
public:
	BoolOption* out_quiet;
	BoolOption* out_verbose;

	BoolOption* out_debug;
	BoolOption* out_names;
	BoolOption* out_short;
	BoolOption* out_full;
	BoolOption* out_tagcoll;
	IntOption* out_limit;
	IntOption* out_cutoff;
	StringOption* out_sort;

	StringOption* filter_tagexpr;
	StringOption* filter_type;

#if 0
	BoolOption* out_facets;
	BoolOption* match_invert;

	BoolOption* misc_local;
	BoolOption* misc_reindex;
	IntOption* misc_distance;
	StringOption* misc_vocfile;

	BoolOption* smse_reltags;
	BoolOption* smse_disctags;

#endif
	Engine* show;
	Engine* search;
	Engine* dumpavail;
	Engine* reindex;
	Engine* info;
	Engine* related;
#if 0
	Engine* update;
	Engine* selfcheck;
	Engine* check;
	Engine* tagcat;
	Engine* tagshow;
	Engine* tagsearch;
	Engine* cat;
	Engine* grep;
	Engine* install;
	Engine* diff;
	Engine* maintainers;
	Engine* tag;
	Engine* submit;
	Engine* todo;
	Engine* score;
	Engine* stats;
	Engine* todoreport;
	Engine* smartsearch;
	Engine* vocfilter;
#endif

	EptCacheOptions() 
		: StandardParserWithMandatoryCommand("ept-cache", VERSION, 1, "enrico@enricozini.org")
	{
		usage = "<command> [options and arguments]";
		description = "High-level tool to access package information";

		// Output options
		out_verbose = add<BoolOption>("verbose", 'v', "verbose", "",
						"enable verbose output");
		out_debug = add<BoolOption>("debug", 0, "debug", "",
						"enable debugging output (including verbose output)");

		// Create the package output group
		OptionGroup* pkgOutputOpts = createGroup("Output options");
		out_names = pkgOutputOpts->add<BoolOption>("names", 0, "names", "",
						"output only the names of the packages");
		out_quiet = pkgOutputOpts->add<BoolOption>("quiet", 'q', "quiet", "",
						"do not write anything to standard output");
		out_short = pkgOutputOpts->add<BoolOption>("short", 0, "short", "",
						"output the names of the packages, plus a short description");
		out_full = pkgOutputOpts->add<BoolOption>("full", 0, "full", "",
						"output the full record of package data");
		out_tagcoll = pkgOutputOpts->add<BoolOption>("tagcoll", 0, "tagcoll", "",
						"tagcoll-style output");
		out_limit = pkgOutputOpts->add<IntOption>("limit", 0, "limit", "count",
						"maximum number of packages to show");
		out_cutoff = pkgOutputOpts->add<IntOption>("cutoff", 0, "cutoff", "percent",
						"do not show results that are this percent worse than the top result");
		out_sort = pkgOutputOpts->add<StringOption>("sort", 's', "sort", "method",
						"sort order (use 'list' for a list of supported options)");

		OptionGroup* filterOpts = createGroup("Filter options");
		filter_tagexpr = filterOpts->add<StringOption>("ftags", 0, "ftags", "tagexpr",
				"only print packages matching this tag expression");
		filter_type = filterOpts->add<StringOption>("type", 't', "type", "name",
				"only print packages of a given type (use 'list' for a list of supported types)");

#if 0
		// Create the collection output group
		OptionGroup* collOutputOpts = createGroup("Options controlling transformations of tag data on output");
		out_facets = collOutputOpts->add<BoolOption>("facets", 0, "facets", "",
						"output only the names of the facets (mainly used for computing statistics)");

		// Create the matching options group
		OptionGroup* matchOpts = createGroup("Options controlling matching of packages");
		match_invert = matchOpts->add<BoolOption>("invert", 'i', "invert", "",
				"invert the match, selecting non-matching items");

		selfcheck = addEngine("selfcheck", "",
			"perform a series of internal self checks using the current tag data");

		check = addEngine("check", "<file>", 
			"check that all the tags in the given tagged collection are present "
			"in the tag vocabulary.  Checks the main database if no file is "
			"specified");

		tagcat = addEngine("tagcat", "", "output the tag vocabulary");

		tagshow = addEngine("tagshow", "", 
			"show the vocabulary informations about a tag");

		tagsearch = addEngine("tagsearch", "<string [string [string ...]]>",
			"show a summary of all tags whose data contains the given strings");

		related = addEngine("related", "<pkg1[,pkg2[,pkg3...]]>",
			"show packages related to the given one(s)",
			"Output a list of the packages that are related to the given package or list of packages.  "
			"If more than one package are to be specified, separate them with commas.\n"
			"The --distance option can be used to control how closely related the output "
			"packages should be from the package(s) specified.");
		related->examples = "debtags related mutt,mozilla-browser";
		misc_distance = related->add<IntOption>("distance", 'd', "distance",
			"set the maximum distance to use for the \"related\" command (defaults to 0)");
			
		cat = addEngine("cat", "", "output the full package tag database");
		cat->add(matchOpts);
		cat->add(collOutputOpts);
#endif

		show = addEngine("show", "<pkg>",
			"show informations about a package, like apt-cache show does, but "
			"adds new fields with all available extra metadata");
		show->add(pkgOutputOpts);

		search = addEngine("search", "[keywords]",
			"output the names and descriptions of the packages that match"
			" the given tag expression");
		search->add(pkgOutputOpts);
		search->add(filterOpts);

		dumpavail = addEngine("dumpavail", "[keywords]",
			"output the full package database, with all extra metadata");
		dumpavail->add(pkgOutputOpts);
		dumpavail->add(filterOpts);

		reindex = addEngine("reindex", "",
			"updates the various indexes managed by libept (requires root).\n"
			"It needs to be run as root");
		reindex->add(out_quiet);

		info = addEngine("info", "",
			"show information about the data providers.\n");

		related = addEngine("related", "[pkgnames]",
			"show packages similar to the given ones",
			"Show packages similar to the given ones.  Package similarity is "
			" computed by how many common elements there are in their"
			" descriptions and their tags");
		related->add(pkgOutputOpts);
		related->add(filterOpts);

#if 0
		update = addEngine("update", "",
			"updates the package tag database (requires root)",
			"Collect package tag data from the sources listed in "
			"/etc/debtags/sources.list, then regenerate the debtags "
			"tag database and main index.\n"
			"It needs to be run as root");
		misc_local = update->add<BoolOption>("local", 0, "local",
			"do not download files when performing an update");
		misc_reindex = update->add<BoolOption>("reindex", 0, "reindex",
			"do not download any file, just do reindexing if needed");
#endif

#if 0
		grep = addEngine("grep", "<tag expression>",
			"output the lines of the full package tag database that match"
			" the given tag expression");
		grep->add(matchOpts);
		grep->add(collOutputOpts);

		install = addEngine("install", "<tag expression>",
			"apt-get install the packages that match the given tag expression",
			"Invokes apt-get install with the names of the packages matched "
			"by the given tag expression.  If you want to see what packages "
			"would be installed you can use debtags search, as "
			"debtags install just calls apt-get install on all "
			"the results of an equivalent debtags search.  Please note "
			"that debtags install is just a prototype feature useful "
			"for experimenting in some environments like Custom Debian "
			"Distributions.  For this reason it is suggested that you "
			"use debtags just as a way to find packages, and "
			"proper package managers as the way to install them");
		install->add(matchOpts);

		diff = addEngine("diff", "[filename]",
			"create a tag patch between the current tag database and the tag"
			"collection [filename].  Standard input is used if filename is not specified");
		diff->aliases.push_back("mkpatch");

		maintainers = addEngine("maintainers", "", 
			"create a tagged collection of maintainers and the tags of the"
			"packages they maintain");
		maintainers->add(collOutputOpts);

		tag = addEngine("tag", "{add|rm|ls} <package> [tags...]",
			"view and edit the tags for a package",
			"General manipulation of tags, useful for automation in scripts.\n"
			"It can be used in three ways:\n"
			"tag add <package> <tags...> will add the tags to the given package\n"
			"tag rm <package> <tags...> will remove the tags from the given package\n"
			"tag ls <package> will output the names of the tags of the given package");

		submit = addEngine("submit", "[patch]",
			"mail the given patch file to the central tag repository."
			"If [patch] is omitted, mail the local tag modifications.");

		todo = addEngine("todo", "", 
			"print a list of the installed packages that are not yet tagged");
		todo->add(pkgOutputOpts);

		score = addEngine("score", "", 
			"score uninstalled packages according to how often their tags "
			"appear in the packages that are installed already");

		stats = addEngine("stats", "",
			"print statistics about Debtags");

		todoreport = addEngine("todoreport", "", 
			"print a report of packages needing work");

		smartsearch = addEngine("smartsearch", "<word [word1 [+tag [-tag1 ...]]]>",
			"Perform a keyword search integrated with related packages.\n"
			"A + prefix indicates a wanted tag.  A - prefix indicates "
			"an unwanted tag.  Other words indicate keywords to search.\n"
			"Remember to use '--' before unwanted tags to avoid to have "
			"them interpreted as commandline switches.\n");
		smse_reltags = smartsearch->add<BoolOption>("relevant", 0, "relevant",
			"only print the tag names sorted by increasing relevance");
		smse_disctags = smartsearch->add<BoolOption>("discriminant", 0, "discriminant",
			"only print the tag names sorted by increasing discriminance");

		vocfilter = addEngine("vocfilter", "tagfile", 
			"filter out the tags that are not found in the given vocabulary file");
		misc_vocfile = vocfilter->add<StringOption>("vocabulary", 0, "vocabulary",
			"vocabulary file to use instead of the current debtags vocabulary");
#endif
	}
};

}
}

// vim:set ts=4 sw=4:
#endif
