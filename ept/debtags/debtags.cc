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

#include <tagcoll/patch.h>
#include <tagcoll/coll/simple.h>
#include <tagcoll/input/stdio.h>
#include <tagcoll/TextFormat.h>

#include <wibble/sys/fs.h>
#include <wibble/string.h>

#include <iostream>
#include <sstream>

#include <sys/wait.h>	// WIFEXITED WEXITSTATUS
#include <sys/types.h>	// getpwuid, getuid
#include <pwd.h>	// getpwuid
#include <unistd.h>	// getuid


using namespace std;
using namespace tagcoll;
using namespace wibble;

namespace ept {
namespace debtags {

Debtags::Debtags(bool editable)
    : m_timestamp(0)
{
    string src = pathname();
    if (!sys::fs::exists(src))
        return;

    // Read uncompressed data
    tagcoll::input::Stdio in(src);

    // Read the collection
    tagcoll::textformat::parse(in, inserter(*this));

    // Read the timestamp
    m_timestamp = sys::fs::timestamp(src, 0);
}

string Debtags::pathname()
{
    const char* res = getenv("DEBTAGS_TAGS");
    if (!res) res = "/var/lib/debtags/package-tags";
    return res;
}

tagcoll::PatchList<std::string, std::string> Debtags::changes() const
{
    return tagcoll::PatchList<std::string, std::string>();
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
	changes().output(tagcoll::inserter(spatch));
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

void Debtags::sendPatch()
{
	PatchList<std::string, std::string> spatch;
	changes().output(tagcoll::inserter(spatch));
	if (!spatch.empty())
	{
		sendPatch(spatch);
	}
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

}
}

#include <tagcoll/patch.tcc>
#include <tagcoll/coll/simple.tcc>
#include <tagcoll/coll/fast.tcc>
#include <tagcoll/TextFormat.tcc>
//#include <tagcoll/stream/filters.tcc>

// Explicit template instantiations for our stuff
template class tagcoll::coll::Fast<std::string, std::string>;

// vim:set ts=4 sw=4:
