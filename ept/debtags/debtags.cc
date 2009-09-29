/** -*- C++ -*-
 * @file
 * @author Enrico Zini (enrico) <enrico@enricozini.org>
 */

/*
 * System tag database
 *
 * Copyright (C) 2003-2008  Enrico Zini <enrico@debian.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include <ept/debtags/debtags.h>
#include <ept/debtags/maint/path.h>
#include <ept/debtags/maint/serializer.h>
#include <ept/debtags/maint/debtagsindexer.h>

#include <tagcoll/input/stdio.h>
#include <tagcoll/TextFormat.h>

#include <wibble/sys/fs.h>
#include <wibble/string.h>

#include <iostream>
#include <sstream>

#include <sys/wait.h>	// WIFEXITED WEXITSTATUS
#include <sys/types.h>	// getpwuid, stat, mkdir, getuid
#include <sys/stat.h>	// stat, mkdir
#include <pwd.h>	// getpwuid
#include <unistd.h>	// stat, getuid


using namespace std;
using namespace tagcoll;
using namespace wibble;

namespace ept {
namespace debtags {

Debtags::Debtags(bool editable)
	: m_coll(m_rocoll)
{
	std::string tagfname;
	std::string idxfname;

	if (!DebtagsIndexer::obtainWorkingDebtags(vocabulary(), tagfname, idxfname))
	{
		m_timestamp = 0;
		return;
	} else {
		m_timestamp = Path::timestamp(idxfname);

		mastermmap.init(idxfname);

		// Initialize the readonly index
		m_pkgid.init(mastermmap, 0);
		m_rocoll.init(mastermmap, 1, 2);
	}

	// Initialize the patch collection layer
	rcdir = Path::debtagsUserSourceDir();

	string patchFile = str::joinpath(rcdir, "patch");
	if (Path::access(patchFile, F_OK) == 0)
	{
		input::Stdio in(patchFile);
		PatchList<int, int> patch;
		textformat::parsePatch(in, patchStringToInt(m_pkgid, vocabulary(), inserter(patch)));
		m_coll.setChanges(patch);
	}
}

tagcoll::PatchList<std::string, Tag> Debtags::changes() const
{
	tagcoll::PatchList<int, int> patches = m_coll.changes();
	tagcoll::PatchList<std::string, Tag> res;

	for (tagcoll::PatchList<int, int>::const_iterator i = patches.begin();
			i != patches.end(); ++i)
	{
		std::string pkg = packageByID(i->second.item);
		if (pkg.empty())
			continue;

		res.addPatch(tagcoll::Patch<std::string, Tag>(pkg,
			vocabulary().tagsByID(i->second.added),
			vocabulary().tagsByID(i->second.removed)));
	}

	return res;
}


#if 0
bool Debtags::hasTagDatabase()
{
	if (Path::access(Path::tagdb(), R_OK) == -1)
	{
		std::cerr << "Missing tag database " << Path::tagdb() << std::endl;
		return false;
	}
	if (Path::access(Path::tagdbIndex(), R_OK) == -1)
	{
		std::cerr << "Missing tag database index " << Path::tagdbIndex() << std::endl;
		return false;
	}
	if (Path::access(Path::vocabulary(), R_OK) == -1)
	{
		std::cerr << "Missing tag vocabulary " << Path::vocabulary() << std::endl;
		return false;
	}
	if (Path::access(Path::vocabularyIndex(), R_OK) == -1)
	{
		std::cerr << "Missing index for tag vocabulary " << Path::vocabularyIndex() << std::endl;
		return false;
	}
	return true;
}
#endif


void Debtags::savePatch()
{
	PatchList<std::string, std::string> spatch;
	m_coll.changes().output(patchIntToString(m_pkgid, vocabulary(), tagcoll::inserter(spatch)));
	savePatch(spatch);
}

void Debtags::savePatch(const PatchList<std::string, std::string>& patch)
{
	std::string patchFile = str::joinpath(rcdir, "patch");
	std::string backup = patchFile + "~";

	wibble::sys::fs::mkFilePath(patchFile);

	if (access(patchFile.c_str(), F_OK) == 0)
		if (rename(patchFile.c_str(), backup.c_str()) == -1)
			throw wibble::exception::System("Can't rename " + patchFile + " to " + backup);

	try {
		FILE* out = fopen(patchFile.c_str(), "w");
		if (out == 0)
			throw wibble::exception::System("Can't write to " + patchFile);

		textformat::outputPatch(patch, out);

		fclose(out);
	} catch (std::exception& e) {
		if (rename(backup.c_str(), patchFile.c_str()) == -1)
            std::cerr << "Warning: Cannot restore previous backup copy: " << e.what() << std::endl;
		throw;
	}
}

void Debtags::savePatch(const PatchList<std::string, Tag>& patch)
{
	PatchList<std::string, std::string> spatch;
	// patch.output(patchToString<C>(m_pkgs, m_pkgidx, m_tags, tagcoll::inserter(spatch)));
	savePatch(spatch);
}

void Debtags::sendPatch()
{
	PatchList<std::string, std::string> spatch;
	m_coll.changes().output(patchIntToString(m_pkgid, vocabulary(), tagcoll::inserter(spatch)));
	if (!spatch.empty())
	{
		sendPatch(spatch);
	}
}

void Debtags::sendPatch(const PatchList<std::string, Tag>& patch)
{
	PatchList<std::string, std::string> spatch;
	// patch.output(patchToString<C>(m_pkgs, m_pkgidx, m_tags, tagcoll::inserter(spatch)));
	sendPatch(spatch);
}

void Debtags::sendPatch(const PatchList<std::string, std::string>& patch)
{
	static const char* cmd = "/usr/sbin/sendmail -t";
	FILE* out = popen(cmd, "w");
	if (out == 0)
		throw wibble::exception::System(std::string("trying to run `") + cmd + "'");

	struct passwd* udata = getpwuid(getuid());

	fprintf(out,
			"To: enrico-debtags@debian.org\n"
			"Bcc: %s\n"
			"Subject: Tag patch\n"
			"Mime-Version: 1.0\n"
			"Content-Type: multipart/mixed; boundary=\"9amGYk9869ThD9tj\"\n"
			"Content-Disposition: inline\n"
			"X-Mailer: debtags-edit\n\n"
			"This mail contains a Debtags patch for the central archive\n\n"
			"--9amGYk9869ThD9tj\n"
			"Content-Type: text/plain; charset=utf-8\n"
			"Content-Disposition: inline\n\n"
			"-- DEBTAGS DIFF V0.1 --\n", udata->pw_name);

	textformat::outputPatch(patch, out);

	fprintf(out, "\n--9amGYk9869ThD9tj\n");

	int res = pclose(out);
	if (!WIFEXITED(res) || WEXITSTATUS(res) != 0)
	{
		std::stringstream str;
		str << res;
		throw wibble::exception::Consistency("checking mailer exit status", "sendmail returned nonzero (" + str.str() + "): the mail may have not been sent");
	}
}


template<typename OUT>
void Debtags::outputSystem(const OUT& cons)
{
	m_rocoll.output(intToPkg(m_pkgid, vocabulary(), cons));
}

template<typename OUT>
void Debtags::outputPatched(const OUT& cons)
{
	m_coll.output(intToPkg(m_pkgid, vocabulary(), cons));
}

}
}

#include <tagcoll/patch.tcc>
#include <tagcoll/coll/patched.tcc>
#include <tagcoll/TextFormat.tcc>
//#include <tagcoll/stream/filters.tcc>

// vim:set ts=4 sw=4:
