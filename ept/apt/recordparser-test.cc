#include "ept/test.h"
#include "recordparser.h"

using namespace ept::tests;
using namespace std;
using namespace ept;
using namespace ept::apt;

namespace {

const char* test_record =
    "A:\n"
    "D: da de di do du\n"
    "B: b\n"
    "C: c \n"
    "Desc: this is the beginning\n"
    " this is the continuation\n"
    " this is the end\n";

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("parsing", []() {
            // Check that the fields are identified and broken up correctly
            RecordParser p(test_record);

            wassert(actual(p.record()) == test_record);
            wassert(actual(p.size()) == 5u);
        });

        add_method("field_tuples", []() {
            RecordParser p(test_record);
            wassert(actual(p.field(0)) == "A:\n");
            wassert(actual(p.field(1)) == "D: da de di do du\n");
            wassert(actual(p.field(2)) == "B: b\n");
            wassert(actual(p.field(3)) == "C: c \n");
            wassert(actual(p.field(4)) == "Desc: this is the beginning\n this is the continuation\n this is the end\n");
        });

        add_method("field_keys", []() {
            RecordParser p(test_record);
            wassert(actual(p.name(0)) == "A");
            wassert(actual(p.name(1)) == "D");
            wassert(actual(p.name(2)) == "B");
            wassert(actual(p.name(3)) == "C");
            wassert(actual(p.name(4)) == "Desc");
        });

        add_method("field_values", []() {
            RecordParser p(test_record);
            wassert(actual(p[0]) == "");
            wassert(actual(p[1]) == "da de di do du");
            wassert(actual(p[2]) == "b");
            wassert(actual(p[3]) == "c");
            wassert(actual(p[4]) == "this is the beginning\n this is the continuation\n this is the end");
        });

        add_method("find_byname", []() {
            // Check that the field search by name finds all the fields
            RecordParser p(test_record);

            wassert(actual(p.index("A")) == 0u);
            wassert(actual(p.index("D")) == 1u);
            wassert(actual(p.index("B")) == 2u);
            wassert(actual(p.index("C")) == 3u);
            wassert(actual(p.index("Desc")) == 4u);

            wassert(actual(p.name(p.index("A"))) == "A");
            wassert(actual(p.name(p.index("B"))) == "B");
            wassert(actual(p.name(p.index("C"))) == "C");
            wassert(actual(p.name(p.index("D"))) == "D");
            wassert(actual(p.name(p.index("Desc"))) == "Desc");
        });

        add_method("indexing", []() {
            RecordParser p(test_record);
            wassert(actual(p["A"]) == "");
            wassert(actual(p["B"]) == "b");
            wassert(actual(p["C"]) == "c");
            wassert(actual(p["D"]) == "da de di do du");
            wassert(actual(p["Desc"]) == "this is the beginning\n this is the continuation\n this is the end");
        });

        add_method("missing_behaviour", []() {
            RecordParser p(test_record);
            // Missing fields give empty strings
            wassert(actual(p.field(100)) == "");
            wassert(actual(p.name(100)) == "");
            wassert(actual(p[100]) == "");
            wassert(actual(p["Missing"]) == "");
        });

        add_method("rescan", []() {
            // Check that scanning twice replaces the old fields
            std::string record =
                "A: a\n"
                "B: b\n"
                "C: c\n";

            RecordParser p(record);
            wassert(actual(p.size()) == 3u);
            wassert(actual(p["A"]) == "a");
            wassert(actual(p["B"]) == "b");
            wassert(actual(p["C"]) == "c");

            std::string record1 =
                "Foo: bar\n"
                "A: different\n";

            p.scan(record1);

            //for (size_t i = 0; i < p.size(); ++i)
            //      cerr << ">> " << i << "==" << p.index(p.name(i)) << " " << p.name(i) << " " << p[i] << endl;

            wassert(actual(p.size()) == 2u);
            wassert(actual(p["A"]) == "different");
            wassert(actual(p["B"]) == "");
            wassert(actual(p["C"]) == "");
            wassert(actual(p["Foo"]) == "bar");
        });

        add_method("real_life", []() {
            // Real-life example
            string record = 
                "Package: apt\n"
                "Priority: important\n"
                "Section: admin\n"
                "Installed-Size: 4368\n"
                "Maintainer: APT Development Team <deity@lists.debian.org>\n"
                "Architecture: amd64\n"
                "Version: 0.6.46.4-0.1\n"
                "Replaces: libapt-pkg-doc (<< 0.3.7), libapt-pkg-dev (<< 0.3.7)\n"
                "Provides: libapt-pkg-libc6.3-6-3.11\n"
                "Depends: libc6 (>= 2.3.5-1), libgcc1 (>= 1:4.1.1-12), libstdc++6 (>= 4.1.1-12), debian-archive-keyring\n"
                "Suggests: aptitude | synaptic | gnome-apt | wajig, dpkg-dev, apt-doc, bzip2\n"
                "Filename: pool/main/a/apt/apt_0.6.46.4-0.1_amd64.deb\n"
                "Size: 1436478\n"
                "MD5sum: 1776421f80d6300c77a608e77a9f4a15\n"
                "SHA1: 1bd7337d2df56d267632cf72ac930c0a4895898f\n"
                "SHA256: b92442ab60046b4d0728245f39cc932f26e17db9f7933a5ec9aaa63172f51fda\n"
                "Description: Advanced front-end for dpkg\n"
                " This is Debian's next generation front-end for the dpkg package manager.\n"
                " It provides the apt-get utility and APT dselect method that provides a\n"
                " simpler, safer way to install and upgrade packages.\n"
                " .\n"
                " APT features complete installation ordering, multiple source capability\n"
                " and several other unique features, see the Users Guide in apt-doc.\n"
                "Build-Essential: yes\n"
                "Tag: admin::package-management, filetransfer::ftp, filetransfer::http, hardware::storage:cd, interface::commandline, network::client, protocol::{ftp,http,ipv6}, role::program, suite::debian, use::downloading, use::searching, works-with::software:package\n";
            RecordParser p(record);

            wassert(actual(p.size()) == 19u);

            string rec1;
            for (size_t i = 0; i < p.size(); ++i)
                rec1 += p.field(i);
            wassert(actual(record) == rec1);
        });

        add_method("buffer_termination", []() {
            // Various buffer termination patterns
            std::string record =
                "A: a\n"
                "B: b";

            RecordParser p(record);
            wassert(actual(p.size()) == 2u);
            wassert(actual(p["A"]) == "a");
            wassert(actual(p["B"]) == "b");
        });

        add_method("buffer_termination2", []() {
            std::string record =
                "A: a\n"
                "B: b\n\n";

            RecordParser p(record);
            wassert(actual(p.size()) == 2u);
            wassert(actual(p["A"]) == "a");
            wassert(actual(p["B"]) == "b");
        });

        add_method("buffer_termination3", []() {
            std::string record =
                "A: a\n"
                "B: b\n\n"
                "C: c\n";

            RecordParser p(record);
            wassert(actual(p.size()) == 2u);
            wassert(actual(p["A"]) == "a");
            wassert(actual(p["B"]) == "b");
        });
    }
} tests("apt_recordparser");

}
