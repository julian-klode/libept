#ifndef EPT_DEBTAGS_VOCABULARYINDEXER_H
#define EPT_DEBTAGS_VOCABULARYINDEXER_H

/** @file
 * @author Enrico Zini <enrico@enricozini.org>
 * Debtags vocabulary indexer
 */

/*
 * Copyright (C) 2003,2004,2005,2006,2007  Enrico Zini <enrico@debian.org>
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

#include <ept/debtags/maint/sourcedir.h>
#include <string>

namespace ept {
namespace debtags {

/**
 * Infrastructure used to rebuild the vocabulary index when needed
 */
struct VocabularyIndexer
{
	SourceDir mainSource;
	SourceDir userSource;
	time_t ts_main_src;
	time_t ts_user_src;
	time_t ts_main_voc;
	time_t ts_main_idx;
	time_t ts_user_voc;
	time_t ts_user_idx;

	/**
	 * Get the timestamp of the newest vocabulary data source
	 */
	time_t sourceTimestamp() const { return ts_main_src < ts_user_src ? ts_user_src : ts_main_src; }
	/**
	 * Return true if the vocabulary index needs rebuilding
	 */
	bool needsRebuild() const;

	/**
	 * Rebuild the vocabulary index
	 * @param vocfname
	 *   Full pathname of the merged vocabulary to create
	 * @param idxfname
	 *   Full pathname of the vocabulary index to create
	 */
	bool rebuild(const std::string& vocfname, const std::string& idxfname);

	/**
	 * Rebuild the vocabulary if needed
	 */
	bool rebuildIfNeeded();

	/**
	 * Get the names of the merged vocabulary and vocabulary index that can be
	 * used to access Debtags vocabulary data.
	 *
	 * The system or the user index will be returned according to which one is
	 * up to date.
	 */
	bool getUpToDateVocabulary(std::string& vocfname, std::string& idxfname);

	/**
	 * Returns true if the index in the user home directory is redundant and
	 * can be deleted.
	 *
	 * The user index is redundant if the system index is up to date.
	 */
	bool userIndexIsRedundant() const;

	/**
	 * Deletes the user index if it is redundant
	 */
	bool deleteRedundantUserIndex();

	/**
	 * Rescan the various timestamps
	 */
	void rescan();

	VocabularyIndexer();

	/**
	 * Get the names of the merged vocabulary and vocabulary index that can be
	 * used to access Debtags vocabulary data.
	 *
	 * The system or the user index will be returned according to which one is
	 * up to date.
	 *
	 * The files will be built or rebuilt if they are missing or outdated.
	 */
	static bool obtainWorkingVocabulary(std::string& vocfname, std::string& idxfname);
};


}
}

// vim:set ts=4 sw=4:
#endif
