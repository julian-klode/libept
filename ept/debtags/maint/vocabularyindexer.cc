#include <ept/debtags/maint/vocabularyindexer.h>
#include <ept/debtags/vocabulary.h>
#include <ept/debtags/maint/vocabularymerger.h>
#include <ept/debtags/maint/path.h>
#include <cstdio>

namespace ept {
namespace debtags {

VocabularyIndexer::VocabularyIndexer()
	: mainSource(Path::debtagsSourceDir()), userSource(Path::debtagsUserSourceDir())
{
	rescan();
}

void VocabularyIndexer::rescan()
{
	ts_main_src = mainSource.vocTimestamp();
	ts_user_src = userSource.vocTimestamp();
	ts_main_voc = Path::timestamp(Path::vocabulary());
	ts_main_idx = Path::timestamp(Path::vocabularyIndex());
	ts_user_voc = Path::timestamp(Path::userVocabulary());
	ts_user_idx = Path::timestamp(Path::userVocabularyIndex());
}

bool VocabularyIndexer::needsRebuild() const
{
	// If there are no indexes of any kind, then we need rebuilding
	if (ts_user_voc == 0 && ts_user_idx == 0 && ts_main_voc == 0 && ts_main_idx == 0)
		return true;

	// If the user index is ok, then we are fine
	if (ts_user_voc >= sourceTimestamp() && ts_user_idx >= sourceTimestamp())
		return false;

	// If there are user sources, then we cannot use the system index
	if (ts_user_src > 0)
		return true;

	// If there are no user sources, then we can fallback on the system
	// indexes in case the user indexes are not up to date
	if (ts_main_voc >= sourceTimestamp() && ts_main_idx >= sourceTimestamp())
		return false;

	return true;
}

bool VocabularyIndexer::userIndexIsRedundant() const
{
	// If there is no user index, then it is not redundant
	if (ts_user_voc == 0 && ts_user_idx == 0)
		return false;

	// If we have user sources, then the user index is never redundant
	if (ts_user_src > 0)
		return false;

	// If the system index is not up to date, then the user index is not
	// redundant
	if (ts_main_voc < sourceTimestamp() || ts_main_idx < sourceTimestamp())
		return false;

	return true;
}

bool VocabularyIndexer::rebuild(const std::string& vocfname, const std::string& idxfname)
{
	using namespace tagcoll;

	// Create the master MMap index
	diskindex::MasterMMapIndexer master(idxfname);

	// Read and merge vocabulary data
	VocabularyMerger voc;
	mainSource.readVocabularies(voc);
	userSource.readVocabularies(voc);

	if (voc.empty())
		return false;
		//throw wibble::exception::Consistency("Reading debtags sources from " + mainSource.path() + " and " + userSource.path(), "Unable to find any vocabulary data");

	// Write the merged vocabulary, and generate tag and facet IDs as a side
	// effect
	std::string tmpvocfname = vocfname + ".tmp";
	voc.write(tmpvocfname);

	// Add the indexed vocabulary data to the master index
	// 0: facets
	master.append(voc.facetIndexer());
	// 1: tags
	master.append(voc.tagIndexer());

	if (rename(tmpvocfname.c_str(), vocfname.c_str()) == -1)
		throw wibble::exception::System("renaming " + tmpvocfname + " to " + vocfname);

	master.commit();
	return true;
}

bool VocabularyIndexer::rebuildIfNeeded()
{
	if (needsRebuild())
	{
		// Decide if we rebuild the user index or the system index

		if (ts_user_src == 0 && Path::access(Path::debtagsIndexDir(), W_OK) == 0)
		{
			// There are no user sources and we can write to the system index
			// directory: rebuild the system index
			if (!rebuild(Path::vocabulary(), Path::vocabularyIndex()))
				return false;
			ts_main_voc = Path::timestamp(Path::vocabulary());
			ts_main_idx = Path::timestamp(Path::vocabularyIndex());
			if (Path::vocabulary() == Path::userVocabulary())
				ts_user_voc = ts_main_voc;
			if (Path::vocabularyIndex() == Path::userVocabularyIndex())
				ts_user_idx = ts_main_idx;
		} else {
			wibble::sys::fs::mkFilePath(Path::userVocabulary());
			wibble::sys::fs::mkFilePath(Path::userVocabularyIndex());
			if (!rebuild(Path::userVocabulary(), Path::userVocabularyIndex()))
				return false;
			ts_user_voc = Path::timestamp(Path::userVocabulary());
			ts_user_idx = Path::timestamp(Path::userVocabularyIndex());
		}
		return true;
	}
	return false;
}

bool VocabularyIndexer::deleteRedundantUserIndex()
{
	if (userIndexIsRedundant())
	{
		// Delete the user indexes if they exist
		if (Path::vocabulary() != Path::userVocabulary())
		{
			unlink(Path::userVocabulary().c_str());
			ts_user_voc = 0;
		}
		if (Path::vocabularyIndex() != Path::userVocabularyIndex())
		{
			unlink(Path::userVocabularyIndex().c_str());
			ts_user_idx = 0;
		}
		return true;
	}
	return false;
}

bool VocabularyIndexer::getUpToDateVocabulary(std::string& vocfname, std::string& idxfname)
{
	// If there are no indexes of any kind, then we have nothing to return
	if (ts_user_voc == 0 && ts_user_idx == 0 && ts_main_voc == 0 && ts_main_idx == 0)
		return false;

	// If the user index is up to date, use it
	if (ts_user_voc >= sourceTimestamp() &&
		ts_user_idx >= sourceTimestamp())
	{
		vocfname = Path::userVocabulary();
		idxfname = Path::userVocabularyIndex();
		return true;
	}

	// If the user index is not up to date and we have user sources, we cannot
	// fall back to the system index
	if (ts_user_src != 0)
		return false;
	
	// Fallback to the system index
	if (ts_main_voc >= sourceTimestamp() &&
		ts_main_idx >= sourceTimestamp())
	{
		vocfname = Path::vocabulary();
		idxfname = Path::vocabularyIndex();
		return true;
	}
	
	return false;
}


bool VocabularyIndexer::obtainWorkingVocabulary(std::string& vocfname, std::string& idxfname)
{
	VocabularyIndexer v;

	v.rebuildIfNeeded();
	v.deleteRedundantUserIndex();
	return v.getUpToDateVocabulary(vocfname, idxfname);
}

}
}

// vim:set ts=4 sw=4:
