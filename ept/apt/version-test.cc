#include "ept/test.h"
#include "version.h"

using namespace std;
using namespace ept::tests;
using namespace ept::apt;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("invalid", []() {
            // Basic test for invalid version
            Version test;

            wassert(actual(test.name()) == "");
            wassert(actual(test.version()) == "");
            wassert(actual(test.isValid()) == false);

            string p = test.name();

            wassert(actual(p) == string());
        });

        add_method("basic", []() {
            // Basic test for version
            Version test("test", "1.0");

            wassert(actual(test.name()) == "test");
            wassert(actual(test.version()) == "1.0");
            wassert(actual(test.isValid()) == true);

            string p = test.name();

            wassert(actual(p) == "test");

            Version v(p, "1.1");
            wassert(actual(v.name()) == "test");
            wassert(actual(v.version()) == "1.1");
            wassert(actual(v.isValid()) == true);
        });

        add_method("comparison", []() {
            // Comparison semanthics
            Version test("test", "1.0");
            Version test1("test", "1.0");

            wassert_true(test == test1);
            wassert_true(! (test != test1));
            wassert_true(! (test < test1));
            wassert_true(! (test > test1));
            wassert_true(test <= test1);
            wassert_true(test >= test1);


            Version test2("test2", "1.0");

            wassert_true(test != test2);
            wassert_true(test != test2);
            wassert_true(test < test2);
            wassert_true(! (test > test2));
            wassert_true(test <= test2);
            wassert_true(! (test >= test2));


            Version test3("test", "2.0");

            wassert_true(test != test3);
            wassert_true(test != test3);
            wassert_true(test < test3);
            wassert_true(! (test > test3));
            wassert_true(test <= test3);
            wassert_true(! (test >= test3));
        });

        add_method("value_copy", []() {
            // Value-copy semanthics
            Version test("test", "1.0");
            Version test1 = test;

            wassert_true(test == test1);

            Version test2;
            test2 = test;
            wassert_true(test == test2);
            wassert_true(test1 == test2);

            Version test3("test", "1.0");
            wassert_true(test == test3);
            wassert_true(test1 == test3);
            wassert_true(test2 == test3);
        });

        add_method("upstream_version", []() {
            // Extraction of upstream version
            wassert(actual(Version("a", "10.0").upstreamVersion()) == "10.0");
            wassert(actual(Version("a", "10.0-1").upstreamVersion()) == "10.0");
            wassert(actual(Version("a", "10.0~foo.1-1.0").upstreamVersion()) == "10.0~foo.1");
            wassert(actual(Version("a", "1.0:10.0~foo.1-1.0").upstreamVersion()) == "10.0~foo.1");
        });

        add_method("policy_comparison", []() {
            // Debian policy comparison semanthics
            wassert_true(Version("a", "10.0") > Version("a", "2.1"));
            wassert_true(Version("a", "1:10.0") < Version("a", "2:2.1"));
            wassert_true(Version("a", "10.0-1") < Version("a", "10.0-2"));
            wassert_true(Version("a", "10.0-2") > Version("a", "10.0-1"));
            wassert_true(Version("a", "1:10.0-1") <= Version("a", "1:10.0-1"));
            wassert_true(Version("a", "1:10.0-1") >= Version("a", "1:10.0-1"));
            // TODO: add more
        });
    }
} tests("apt_version");

}
