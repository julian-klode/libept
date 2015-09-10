#include "ept/test.h"
#include "axi.h"
#include "ept/apt/apt.h"
#include "ept/utils/sys.h"
#include <set>

using namespace ept::tests;
using namespace std;
using namespace ept;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("empty", []() {
            // Access an empty index
            sys::mkdir_ifmissing("xapian", 0755);
            apt::Apt apt;
            axi::OverrideIndexDir oid("./empty");
            wassert(actual(axi::timestamp()) == 0);
            sys::rmdir("xapian");
        });
    }
} tests("axi");

}
