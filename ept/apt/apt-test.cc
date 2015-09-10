#include "ept/test.h"
#include "apt.h"
#include <set>
#include <algorithm>

using namespace std;
using namespace ept;
using namespace ept::tests;
using namespace ept::apt;

namespace {

struct AptTestEnvironment {
    //ept::core::AptDatabase db;
    AptTestEnvironment() {
        pkgInitConfig (*_config);
        _config->Set("Initialized", 1);
        _config->Set("Dir", ".");
        _config->Set("Dir::Cache", "cache");
        _config->Set("Dir::State", "state");
        _config->Set("Dir::Etc", "etc");
        _config->Set("Dir::Etc::sourcelist", "sources.list");
        _config->Set("Dir::State::status", "./dpkg-status");
        pkgInitSystem (*_config, _system);
    }
};

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("iterators", []() {
            // Check that iterations iterates among some packages
            AptTestEnvironment env;
            Apt apt;
            Apt::iterator i = apt.begin();
            wassert_true(i != apt.end());

            size_t count = 0;
            for (; i != apt.end(); ++i)
                ++count;

            wassert_true(count > 100);
        });

        add_method("apt_exists", []() {
            // Check that iteration gives some well-known packages
            AptTestEnvironment env;
            Apt apt;
            set<string> packages;

            std::copy(apt.begin(), apt.end(), inserter(packages, packages.begin()));

            wassert_true(packages.find("libsp1") != packages.end());
            // TODO this exposes a bug somewhere... sp definitely is among
            // the packages
            // wassert_true(packages.find("sp") != packages.end());
            wassert_true(packages.find("") == packages.end());
        });

        add_method("timestamp", []() {
            // Check that timestamp gives some meaningful timestamp
            AptTestEnvironment env;
            Apt apt;
            time_t ts = apt.timestamp();
            wassert_true(ts > 1000000);
        });

        add_method("validity", []() {
            // Check the package validator
            AptTestEnvironment env;
            Apt apt;
            wassert_true(apt.isValid("apt"));
            wassert_true(!apt.isValid("this-package-does-not-really-exists"));
        });

        add_method("versions", []() {
            // Check the version instantiators
            AptTestEnvironment env;
            Apt apt;
            std::string pkg("apt");
            Version ver = apt.candidateVersion(pkg);
            wassert_true(ver.isValid());

            ver = apt.installedVersion(pkg);
            wassert_true(ver.isValid());

            ver = apt.anyVersion(pkg);
            wassert_true(ver.isValid());

            std::string pkg1("this-package-does-not-really-exists");
            ver = apt.candidateVersion(pkg1);
            wassert_true(!ver.isValid());

            ver = apt.installedVersion(pkg1);
            wassert_true(!ver.isValid());

            ver = apt.anyVersion(pkg1);
            wassert_true(!ver.isValid());
        });

        add_method("version_validity", []() {
            // Check the version validator
            AptTestEnvironment env;
            Apt apt;
            Version ver = apt.candidateVersion("apt");
            wassert_true(apt.validate(ver) == ver);

            ver = Version("this-package-does-not-really-exists", "0.1");
            wassert_true(!apt.validate(ver).isValid());

            ver = Version("apt", "0.31415");
            wassert_true(!apt.validate(ver).isValid());
        });

        add_method("raw_record", []() {
            // Check the raw record accessor
            AptTestEnvironment env;
            Apt apt;
            string pkg("sp");
            Version ver = apt.candidateVersion(pkg);
            wassert_true(ver.isValid());
            wassert_true(apt.validate(ver) == ver);

            string record = apt.rawRecord(ver);
            wassert_true(record.find("Package: sp") != string::npos);
            wassert_true(record.find("Section: text") != string::npos);

            record = apt.rawRecord(Version("sp", "0.31415"));
            wassert(actual(record) == string());

            wassert(actual(apt.rawRecord(pkg)) == apt.rawRecord(apt.anyVersion(pkg)));
        });

        add_method("state", []() {
            // Check the package state accessor
            AptTestEnvironment env;
            Apt apt;
            PackageState s = apt.state("kdenetwork");
            wassert_true(s.isValid());
            wassert_true(s.isInstalled());

            s = apt.state("this-package-does-not-really-exists");
            wassert_true(!s.isValid());
        });

        add_method("record_iteration", []() {
            // Check the record iterator (accessing with *)
            AptTestEnvironment env;
            Apt apt;
            size_t count = 0;
            for (Apt::record_iterator i = apt.recordBegin();
                 i != apt.recordEnd(); ++i)
                {
                    wassert_true((*i).size() > 8);
                    wassert(actual((*i).substr(0, 8)) == "Package:");
                    ++count;
                }
            wassert_true(count > 200);
        });

        add_method("record_iteration2", []() {
            // Check the record iterator (accessing with ->)
            AptTestEnvironment env;
            Apt apt;
            size_t count = 0;
            for (Apt::record_iterator i = apt.recordBegin();
                 i != apt.recordEnd(); ++i)
                {
                    wassert_true(i->size() > 8);
                    wassert(actual(i->substr(0, 8)) == "Package:");
                    ++count;
                }
            wassert_true(count > 200);
        });

        add_method("stl_iteration", []() {
            // Check that the iterators can be used with the algorithms
            AptTestEnvironment env;
            Apt apt;
            vector<string> out;
            std::copy(apt.begin(), apt.end(), back_inserter(out));
        });

        add_method("stl_record_iteration", []() {
            // Check that the iterators can be used with the algorithms
            AptTestEnvironment env;
            Apt apt;
            vector<string> out;
            std::copy(apt.recordBegin(), apt.recordEnd(), back_inserter(out));
        });

        add_method("check_updates", []() {
            // Check that checkUpdates will keep a working Apt object
            AptTestEnvironment env;
            Apt apt;
            wassert_true(apt.isValid("apt"));
            apt.checkCacheUpdates();
            wassert_true(apt.isValid("apt"));
            apt.invalidateTimestamp();
            apt.checkCacheUpdates();
            wassert_true(apt.isValid("apt"));
        });
    }
} tests("apt_apt");

}
