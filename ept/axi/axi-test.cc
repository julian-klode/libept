#include "ept/test.h"
#include "axi.h"
#include "ept/apt/apt.h"
#include <wibble/sys/fs.h>
#include <set>

using namespace ept::tests;
using namespace std;
using namespace ept;

namespace {

struct DirMaker
{
	DirMaker(const std::string& name)
	{
		wibble::sys::fs::mkdirIfMissing(name, 0755);
	}
};

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("empty", []() {
            // Access an empty index
            DirMaker md("xapian");
            apt::Apt apt;
            axi::OverrideIndexDir oid("./empty");
            wassert(actual(axi::timestamp()) == 0);
        });
    }
} tests("axi");

}
