#ifndef EPT_DEBTAGS_SOURCEDIR_TCC
#define EPT_DEBTAGS_SOURCEDIR_TCC

/** @file
 * @author Enrico Zini <enrico@enricozini.org>
 * Debtags data source directory access
 */
#include <ept/debtags/maint/sourcedir.h>

#include <tagcoll/input/zlib.h>
#include <tagcoll/input/stdio.h>
#include <wibble/sys/fs.h>

using namespace wibble;

namespace ept {
namespace debtags {

template<typename OUT>
void SourceDir::readTags(OUT out)
{
    auto_ptr<sys::fs::Directory> dir;
    try {
        dir.reset(new sys::fs::Directory(path));
    } catch (wibble::exception::System& e) {
        return;
    }

    for (sys::fs::Directory::const_iterator d = dir->begin(); d != dir->end(); ++d)
    {
        string name = *d;
        FileType type = fileType(name);
		if (type == TAG)
		{
            // Read uncompressed data
            tagcoll::input::Stdio in(path + "/" + name);

			// Read the collection
			tagcoll::textformat::parse(in, out);
		}
		else if (type == TAGGZ)
		{
            // Read compressed data
            tagcoll::input::Zlib in(path + "/" + name);

			// Read the collection
			tagcoll::textformat::parse(in, out);
		}
	}
}

}
}

#include <tagcoll/TextFormat.tcc>

#endif

// -*- C++ -*-
// vim:set ts=4 sw=4:
