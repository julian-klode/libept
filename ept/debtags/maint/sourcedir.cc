#include <ept/debtags/maint/sourcedir.h>
#include <ept/debtags/vocabulary.h>
#include <ept/debtags/maint/path.h>

#include <wibble/string.h>
#include <wibble/sys/fs.h>

#include <tagcoll/input/zlib.h>
#include <tagcoll/input/stdio.h>

using namespace std;
using namespace wibble;

namespace ept {
namespace debtags {

SourceDir::SourceDir(const std::string& path)
    : sys::fs::Directory(path)
{
}
SourceDir::~SourceDir()
{
}

SourceDir::FileType SourceDir::fileType(const std::string& name)
{
	if (name[0] == '.') return SKIP;

	// Filenames need to be at least 5 characters long (one char plus
	// extension)
	if (name.size() <= 4) return SKIP;

    if (name == "package-tags") return TAG;
    if (name == "vocabulary") return VOC;

	// Only look at .voc and .tag files
	std::string ext(name, name.size() - 4);
	if (ext == ".voc")
		return VOC;
	if (ext == ".tag")
		return TAG;

	// Now look for compressed files, which must have the 4 character extension
	// plus the 3 chars of '.gz'
	if (name.size() <= 7) return SKIP;

	ext = name.substr(name.size() - 7);
	if (ext == ".voc.gz")
		return VOCGZ;
	if (ext == ".tag.gz")
		return TAGGZ;

	return SKIP;
}

time_t SourceDir::timestamp()
{
    if (!exists()) return 0;

    time_t max = 0;
    for (const_iterator d = begin(); d != end(); ++d)
    {
        string name = *d;
        FileType type = fileType(name);
        if (type == SKIP) continue;

        time_t ts = Path::timestamp(str::joinpath(m_path, name));
        if (ts > max) max = ts;
	}

	return max;
}

time_t SourceDir::vocTimestamp()
{
    if (!exists()) return 0;

    time_t max = 0;
    for (const_iterator d = begin(); d != end(); ++d)
    {
        string name = *d;
        FileType type = fileType(name);
        if (type != VOC and type != VOCGZ) continue;

        time_t ts = Path::timestamp(str::joinpath(m_path, name));
        if (ts > max) max = ts;
	}

	return max;
}

time_t SourceDir::tagTimestamp()
{
    if (!exists()) return 0;

    time_t max = 0;
    for (const_iterator d = begin(); d != end(); ++d)
    {
        string name = *d;
        FileType type = fileType(name);
        if (type != TAG and type != TAGGZ) continue;

        time_t ts = Path::timestamp(str::joinpath(m_path, name));
        if (ts > max) max = ts;
	}

	return max;
}

void SourceDir::readVocabularies(Vocabulary& out)
{
    if (!exists()) return;

    for (const_iterator d = begin(); d != end(); ++d)
    {
        string name = *d;
        if (name[0] == '.') continue;
        FileType type = fileType(name);
		if (type == VOC)
		{
            // Read uncompressed data
            tagcoll::input::Stdio in(str::joinpath(m_path, name));

			// Read the vocabulary
			out.read(in);
		}
		else if (type == VOCGZ)
		{
            // Read compressed data
            tagcoll::input::Zlib in(str::joinpath(m_path, name));

			// Read the vocabulary
			out.read(in);
		}
	}
}

}
}

// vim:set ts=4 sw=4:
