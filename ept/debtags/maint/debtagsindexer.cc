#include <ept/debtags/maint/debtagsindexer.h>
#include <ept/debtags/maint/path.h>
#include <ept/debtags/maint/pkgid.h>
#include <ept/debtags/maint/serializer.h>
#include <ept/debtags/vocabulary.h>

#include <tagcoll/coll/intdiskindex.h>
#include <tagcoll/coll/simple.h>
#include <tagcoll/TextFormat.h>
#include <tagcoll/stream/filters.h>

#include <wibble/exception.h>

#include <cstring>

using namespace std;

namespace ept {
namespace debtags {

/// MMapIndexer that indexes the package names
struct PkgIdGenerator : public tagcoll::diskindex::MMapIndexer
{
	// Sorted set of all available package names
	std::set<std::string> pkgs;

	int encodedSize() const
	{
		int size = pkgs.size() * sizeof(int);
		for (std::set<std::string>::const_iterator i = pkgs.begin();
				i != pkgs.end(); ++i)
			size += i->size() + 1;
		return tagcoll::diskindex::MMap::align(size);
	}

	void encode(char* buf) const
	{
		int pos = pkgs.size() * sizeof(int);
		int idx = 0;
		for (std::set<std::string>::const_iterator i = pkgs.begin();
				i != pkgs.end(); ++i)
		{
			((int*)buf)[idx++] = pos;
			memcpy(buf + pos, i->c_str(), i->size() + 1);
			pos += i->size() + 1;
		}
	}
};


DebtagsIndexer::DebtagsIndexer(Vocabulary& voc)
	: voc(voc),
	  mainSource(Path::debtagsSourceDir()),
	  userSource(Path::debtagsUserSourceDir())
{
	rescan();
}

void DebtagsIndexer::rescan()
{
	ts_main_src = mainSource.timestamp();
	ts_user_src = userSource.timestamp();
	ts_main_tag = Path::timestamp(Path::tagdb());
	ts_main_idx = Path::timestamp(Path::tagdbIndex());
	ts_user_tag = Path::timestamp(Path::userTagdb());
	ts_user_idx = Path::timestamp(Path::userTagdbIndex());
}

bool DebtagsIndexer::needsRebuild() const
{
	// If there are no indexes of any kind, then we need rebuilding
	if (ts_user_tag == 0 && ts_user_idx == 0 && ts_main_tag == 0 && ts_main_idx == 0)
		return true;

	// If the user index is ok, then we are fine
	if (ts_user_tag >= sourceTimestamp() && ts_user_idx >= sourceTimestamp())
		return false;

	// If there are user sources, then we cannot use the system index
	if (ts_user_src > 0)
		return true;

	// If there are no user sources, then we can fallback on the system
	// indexes in case the user indexes are not up to date
	if (ts_main_tag >= sourceTimestamp() && ts_main_idx >= sourceTimestamp())
		return false;

	return true;
}

bool DebtagsIndexer::userIndexIsRedundant() const
{
	// If there is no user index, then it is not redundant
	if (ts_user_tag == 0 && ts_user_idx == 0)
		return false;

	// If we have user sources, then the user index is never redundant
	if (ts_user_src > 0)
		return false;

	// If the system index is not up to date, then the user index is not
	// redundant
	if (ts_main_tag < sourceTimestamp() || ts_main_idx < sourceTimestamp())
		return false;

	return true;
}

bool DebtagsIndexer::rebuild(const std::string& tagfname, const std::string& idxfname)
{
	using namespace tagcoll;

	diskindex::MasterMMapIndexer master(idxfname);

	// Read and merge tag data
	coll::Simple<string, string> merged;
	mainSource.readTags(inserter(merged));
	userSource.readTags(inserter(merged));

	if (merged.empty())
		//throw wibble::exception::Consistency("Reading debtags sources from " + Path::debtagsSourceDir() + " and " + Path::debtagsUserSourceDir(), "Unable to find any tag data");
		return false;

	// Create the pkgid index
	PkgIdGenerator pkgidGen;
	for (coll::Simple<string, string>::const_iterator i = merged.begin();
			i != merged.end(); ++i)
		pkgidGen.pkgs.insert(i->first);

	// Temporary in-memory index to use for converting packages to ints while
	// creating the debtags index
	char buf[pkgidGen.encodedSize()];
	pkgidGen.encode(buf);
	PkgId pkgid(buf, pkgidGen.encodedSize());

	// Create the Debtags index
	coll::IntDiskIndexer tagindexer;
	merged.output(stringToInt(pkgid, voc, inserter(tagindexer)));

	// MMap 0: pkgid
	master.append(pkgidGen);
	// MMap 1: pkg->tag
	master.append(tagindexer.pkgIndexer());
	// MMap 2: tag->pkg
	master.append(tagindexer.tagIndexer());

	// Write the tag database in text format
	std::string tmpdb = tagfname + ".tmp";
	FILE* out = fopen(tmpdb.c_str(), "wt");
	if (!out) throw wibble::exception::File(tmpdb, "creating temporary copy of tag index");
	merged.output(textformat::StdioWriter(out));
	fclose(out);

	// Perform "atomic" update of the tag database
	// FIXME: cannot be atomic because race conditions happening between file
	// renames
	if (rename(tmpdb.c_str(), tagfname.c_str()) == -1)
		throw wibble::exception::System("Renaming " + tmpdb + " to " + tagfname);

	master.commit();
	return true;
}

bool DebtagsIndexer::rebuildIfNeeded()
{
	if (needsRebuild())
	{
		// Decide if we rebuild the user index or the system index

		if (ts_user_src == 0 && Path::access(Path::debtagsIndexDir(), W_OK) == 0)
		{
			// There are no user sources and we can write to the system index
			// directory: rebuild the system index
			if (!rebuild(Path::tagdb(), Path::tagdbIndex()))
				return false;
			ts_main_tag = Path::timestamp(Path::tagdb());
			ts_main_idx = Path::timestamp(Path::tagdbIndex());
			if (Path::tagdb() == Path::userTagdb())
				ts_user_tag = ts_main_tag;
			if (Path::tagdbIndex() == Path::userTagdbIndex())
				ts_user_idx = ts_main_idx;
		} else {
			wibble::sys::fs::mkFilePath(Path::userTagdb());
			wibble::sys::fs::mkFilePath(Path::userTagdbIndex());
			if (!rebuild(Path::userTagdb(), Path::userTagdbIndex()))
				return false;
			ts_user_tag = Path::timestamp(Path::userTagdb());
			ts_user_idx = Path::timestamp(Path::userTagdbIndex());
		}
		return true;
	}
	return false;
}

bool DebtagsIndexer::deleteRedundantUserIndex()
{
	if (userIndexIsRedundant())
	{
		// Delete the user indexes if they exist
		if (Path::tagdb() != Path::userTagdb())
		{
			unlink(Path::userTagdb().c_str());
			ts_user_tag = 0;
		}
		if (Path::tagdbIndex() != Path::userTagdbIndex())
		{
			unlink(Path::userTagdbIndex().c_str());
			ts_user_idx = 0;
		}
		return true;
	}
	return false;
}

bool DebtagsIndexer::getUpToDateTagdb(std::string& tagfname, std::string& idxfname)
{
	// If there are no indexes of any kind, then we have nothing to return
	if (ts_user_tag == 0 && ts_user_idx == 0 && ts_main_tag == 0 && ts_main_idx == 0)
		return false;

	// If the user index is up to date, use it
	if (ts_user_tag >= sourceTimestamp() &&
		ts_user_idx >= sourceTimestamp())
	{
		tagfname = Path::userTagdb();
		idxfname = Path::userTagdbIndex();
		return true;
	}

	// If the user index is not up to date and we have user sources, we cannot
	// fall back to the system index
	if (ts_user_src != 0)
		return false;
	
	// Fallback to the system index
	if (ts_main_tag >= sourceTimestamp() &&
		ts_main_idx >= sourceTimestamp())
	{
		tagfname = Path::tagdb();
		idxfname = Path::tagdbIndex();
		return true;
	}
	
	return false;
}



bool DebtagsIndexer::obtainWorkingDebtags(Vocabulary& voc, std::string& tagfname, std::string& idxfname)
{
	DebtagsIndexer t(voc);

	t.rebuildIfNeeded();
	t.deleteRedundantUserIndex();
	return t.getUpToDateTagdb(tagfname, idxfname);
}

}
}

#include <ept/debtags/maint/sourcedir.tcc>
#include <tagcoll/coll/simple.tcc>

// vim:set ts=4 sw=4:
// -*- C++ -*-
