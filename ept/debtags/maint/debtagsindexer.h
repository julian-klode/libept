#ifndef EPT_DEBTAGS_DEBTAGSINDEXER_H
#define EPT_DEBTAGS_DEBTAGSINDEXER_H

#include <ept/debtags/maint/sourcedir.h>
#include <string>

namespace ept {
namespace debtags {

class Vocabulary;

struct DebtagsIndexer
{
	Vocabulary& voc;

	SourceDir mainSource;
	SourceDir userSource;
	time_t ts_main_src;
	time_t ts_user_src;
	time_t ts_main_tag;
	time_t ts_main_idx;
	time_t ts_user_tag;
	time_t ts_user_idx;

	time_t sourceTimestamp() const
	{
		time_t res = ts_main_src;
		if (ts_user_src > res) res = ts_user_src;
		return res;
	}
	bool needsRebuild() const;
	bool rebuild(const std::string& tagfname, const std::string& idxfname);
	bool rebuildIfNeeded();
	bool getUpToDateTagdb(std::string& tagfname, std::string& idxfname);

	bool userIndexIsRedundant() const;
	bool deleteRedundantUserIndex();

	void rescan();

	DebtagsIndexer(Vocabulary& voc);

	static bool obtainWorkingDebtags(Vocabulary& voc, std::string& tagfname, std::string& idxfname);
};


}
}

// vim:set ts=4 sw=4:
#endif
