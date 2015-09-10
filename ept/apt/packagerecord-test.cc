#include "ept/test.h"
#include "packagerecord.h"

namespace std {
    ostream& operator<<(ostream& out, const set<string>& s)
        {
            for (set<string>::const_iterator i = s.begin();
                 i != s.end(); ++i)
                if (i == s.begin())
                    out << *i;
                else
                    out << ", " << *i;
            return out;
        }
}

using namespace std;
using namespace ept;
using namespace ept::tests;
using namespace ept::apt;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("supported_fields", []() {
            // Check that the supported fields are understood
            string record =
                "Package: apt\n"
                "Priority: important\n"
                "Section: admin\n"
                "Installed-Size: 4368\n"
                "Maintainer: APT Development Team <deity@lists.debian.org>\n"
                "Architecture: amd64\n"
                "Source: apt\n"
                "Version: 0.6.46.4-0.1\n"
                "Replaces: libapt-pkg-doc (<< 0.3.7), libapt-pkg-dev (<< 0.3.7)\n"
                "Provides: libapt-pkg-libc6.3-6-3.11\n"
                "Depends: libc6 (>= 2.3.5-1), libgcc1 (>= 1:4.1.1-12), libstdc++6 (>= 4.1.1-12), debian-archive-keyring\n"
                "Pre-Depends: debtags (maybe)\n"
                "Suggests: aptitude | synaptic | gnome-apt | wajig, dpkg-dev, apt-doc, bzip2\n"
                "Recommends: debtags (maybe)\n"
                "Enhances: debian\n"
                "Conflicts: marameo\n"
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

            PackageRecord p(record);

            wassert(actual(p.size()) == 24u);

            wassert(actual(p.package()) == "apt");
            wassert(actual(p.priority()) == "important");
            wassert(actual(p.section()) == "admin");
            wassert(actual(p.installedSize()) == 4368u);
            wassert(actual(p.maintainer()) == "APT Development Team <deity@lists.debian.org>");
            wassert(actual(p.architecture()) == "amd64");
            wassert(actual(p.source()) == "apt");
            wassert(actual(p.version()) == "0.6.46.4-0.1");
            wassert(actual(p.replaces()) == "libapt-pkg-doc (<< 0.3.7), libapt-pkg-dev (<< 0.3.7)");
            wassert(actual(p.provides()) == "libapt-pkg-libc6.3-6-3.11");
            wassert(actual(p.depends()) == "libc6 (>= 2.3.5-1), libgcc1 (>= 1:4.1.1-12), libstdc++6 (>= 4.1.1-12), debian-archive-keyring");
            wassert(actual(p.preDepends()) == "debtags (maybe)");
            wassert(actual(p.recommends()) == "debtags (maybe)");
            wassert(actual(p.suggests()) == "aptitude | synaptic | gnome-apt | wajig, dpkg-dev, apt-doc, bzip2");
            wassert(actual(p.enhances()) == "debian");
            wassert(actual(p.conflicts()) == "marameo");
            wassert(actual(p.filename()) == "pool/main/a/apt/apt_0.6.46.4-0.1_amd64.deb");
            wassert(actual(p.packageSize()) == 1436478u);
            wassert(actual(p.md5sum()) == "1776421f80d6300c77a608e77a9f4a15");
            wassert(actual(p.sha1()) == "1bd7337d2df56d267632cf72ac930c0a4895898f");
            wassert(actual(p.sha256()) == "b92442ab60046b4d0728245f39cc932f26e17db9f7933a5ec9aaa63172f51fda");
            wassert(actual(p.description()) == "Advanced front-end for dpkg\n"
                          " This is Debian's next generation front-end for the dpkg package manager.\n"
                          " It provides the apt-get utility and APT dselect method that provides a\n"
                          " simpler, safer way to install and upgrade packages.\n"
                          " .\n"
                          " APT features complete installation ordering, multiple source capability\n"
                          " and several other unique features, see the Users Guide in apt-doc.");
            wassert(actual(p.shortDescription()) == "Advanced front-end for dpkg");
            wassert(actual(p.longDescription()) ==
                          "This is Debian's next generation front-end for the dpkg package manager.\n"
                          " It provides the apt-get utility and APT dselect method that provides a\n"
                          " simpler, safer way to install and upgrade packages.\n"
                          " .\n"
                          " APT features complete installation ordering, multiple source capability\n"
                          " and several other unique features, see the Users Guide in apt-doc.");
            wassert(actual(p.buildEssential()) == true);
            
            std::set<std::string> tags;
            tags.insert("admin::package-management");
            tags.insert("filetransfer::ftp");
            tags.insert("filetransfer::http");
            tags.insert("hardware::storage:cd");
            tags.insert("interface::commandline");
            tags.insert("network::client");
            tags.insert("protocol::ftp");
            tags.insert("protocol::http");
            tags.insert("protocol::ipv6");
            tags.insert("role::program");
            tags.insert("suite::debian");
            tags.insert("use::downloading");
            tags.insert("use::searching");
            tags.insert("works-with::software:package");
            wassert(actual(p.tag()) == tags);
        });
    }
} tests("apt_packagerecord");

}
