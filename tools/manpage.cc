/*
 * tagged collection - Experimental programs to test and study tagged collections
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
#include <wibble/commandline/doc.h>
#include "EptCacheOptions.h"
#include <iostream>

using namespace std;

int main(int argc, const char* argv[])
{
	try {
		if (argc == 1)
			throw wibble::exception::BadOption("no arguments provided");

		string cmd(argv[1]);
		string hooks(argc > 2 ? argv[2] : "");

		if (cmd == "ept-cache")
		{
			wibble::commandline::EptCacheOptions opts;
			wibble::commandline::Manpage help("ept-cache", VERSION, 1, "enrico@enricozini.org");
			if (!hooks.empty())
				help.readHooks(hooks);
			help.output(cout, opts);
		}
		else
			throw wibble::exception::BadOption("unknown command " + cmd);

		return 0;
	} catch (wibble::exception::BadOption& e) {
		cerr << e.desc() << endl << endl;
		cerr << "Usage: manpage <command>" << endl << endl;
		cerr << "Supported commands are: ept-cache" << endl;
		return 1;
	} catch (std::exception& e) {
		cerr << e.what() << endl;
		return 1;
	}
}

#include <ept/debtags/debtags.tcc>

// vim:set ts=4 sw=4:
