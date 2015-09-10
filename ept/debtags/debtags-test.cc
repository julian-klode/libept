#include "ept/test.h"
#include "debtags.h"
#include "coll/operators.h"
#include <cstdio>

using namespace std;
using namespace ept;
using namespace ept::debtags;
using namespace ept::tests;
using namespace ept::debtags::coll::operators;

#define testfile TEST_ENV_DIR "debtags/package-tags"

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("iterate", []() {
            EnvOverride eo("DEBTAGS_TAGS", testfile);
            Debtags debtags;
            for (Debtags::const_iterator i = debtags.begin(); i != debtags.end(); ++i)
            {
                *i;
                i->first;
                i->second;
            }
        });

        add_method("lookup_tags", []() {
            EnvOverride eo("DEBTAGS_TAGS", testfile);
            Debtags debtags;
            string p("debtags");
            std::set<std::string> tags = debtags.getTagsOfItem(p);
            wassert(actual(tags.empty()).isfalse());

#if 0
            for ( std::set< Tag >::iterator i = tags.begin(); i != tags.end(); ++ i ) {
                std::cerr << i->id() << ": " << i->fullname() << std::endl;
            }
            std::cerr << "---" << std::endl;
            Tag t = voc().tagByName( "interface::commandline" );
            std::cerr << t.id() << ": " << t.fullname() << std::endl;
#endif

            wassert(actual(tags.size()) == 8u);
            wassert(actual(tags.find("devel::buildtools") != tags.end()).istrue());
            wassert(actual(tags.find("implemented-in::c++") != tags.end()).istrue());
            wassert(actual(tags.find("interface::commandline") != tags.end()).istrue());
            wassert(actual(tags.find("role::program") != tags.end()).istrue());
            wassert(actual(tags.find("scope::application") != tags.end()).istrue());
            wassert(actual(tags.find("suite::debian") != tags.end()).istrue());
            wassert(actual(tags.find("use::searching") != tags.end()).istrue());
            wassert(actual(tags.find("works-with::software:package") != tags.end()).istrue());
        });

        add_method("lookup_packages", []() {
            using namespace std;
            EnvOverride eo("DEBTAGS_TAGS", testfile);
            Debtags debtags;

            /* Get the 'debtags' package */
            string p("debtags");

            /* Get its tags */
            std::set<std::string> tags = debtags.getTagsOfItem(p);
            wassert(actual(tags.empty()).isfalse());

            /*
            cerr << "Intersection size: " << endl;
            using namespace wibble::operators;
            std::set<Tag>::const_iterator dbgi = tags.begin();
            cerr << "* " << dbgi->fullname() << ": " << dbgi->id() << endl;
            std::set<int> dbgres = debtags.tagdb().getItemsHavingTag(dbgi->id());
            std::set<Package> dbgpres = debtags.getItemsHavingTag(*dbgi);
            cerr << " #pkgs " << dbgres.size() << " == " << dbgpres.size() << endl;
            cerr << " #isec " << dbgres.size() << " == " << dbgpres.size() << endl;
            cerr << "  "; ppset(dbgpres); cerr << endl;
            cerr << "  "; piset(dbgres); cerr << endl;
            for (++dbgi ; dbgi != tags.end(); ++dbgi)
            {
                cerr << "* " << dbgi->fullname() << ": " << dbgi->id() << endl;
                std::set<Package> dbgpkgs = debtags.getItemsHavingTag(*dbgi);
                std::set<int> dbgids = debtags.tagdb().getItemsHavingTag(dbgi->id());
                cerr << "  "; ppset(dbgpkgs); cerr << endl;
                cerr << "  "; piset(dbgids); cerr << endl;
                cerr << " #pkgs " << dbgpkgs.size() << " == " << dbgids.size() << endl;
                dbgres &= dbgids;
                dbgpres &= dbgpkgs;
                cerr << " #isec " << dbgres.size() << " == " << dbgpres.size() << endl;
            }
            cerr << " " << dbgres.size() << endl << "Results: " << endl;
            for (std::set<int>::const_iterator i = dbgres.begin(); i != dbgres.end(); ++i)
                cerr << "   " << *i << endl;
            */


        //	cerr << "Tags of debtags: ";
        //	for (std::set<Tag>::const_iterator i = tags.begin(); i != tags.end(); ++i)
        //	{
        //		cerr << " " + i->fullname() << endl;
        //		std::set<Package> packages = debtags.getItemsHavingTag(*i);
        //		for (std::set<Package>::const_iterator p = packages.begin();
        //				p != packages.end(); ++p)
        //			cerr << "   PKG " << p->name() << endl;
        //	}
        //	cerr << endl;

            /* Get the items for the tagset of 'debtags' */
            std::set<string> packages = debtags.getItemsHavingTags(tags);
            //cerr << packages.size() << endl;
            wassert(actual(packages.empty()).isfalse());
            /*
            for ( std::set< Package >::iterator i = packages.begin(); i != packages.end(); ++ i )
                std::cerr << i->name() << std::endl;
            std::cerr << "---" << std::endl;
            std::cerr << p.name() << std::endl;
            */
            /* They should at least contain 'debtags' */
            wassert(actual(p <= packages).istrue());

            /* Get one of the tags of 'debtags' */
            std::string tag = *tags.begin();

            /* Get its items */
            {
                /* Need this workaround until I figure out how to tell the new GCC
                 * that TagDB is a TDBReadonlyDiskIndex and should behave as such
                 */
                std::set<std::string> ts;
                ts.insert(tag);
                packages = debtags.getItemsHavingTags(ts);
            }
            //packages = c.debtags().tagdb().getItems(tag);
            wassert(actual(packages.empty()).isfalse());
            /* They should at least contain 'debtags' */
            wassert(actual(p <= packages).istrue());

            //c.debtags().getTags(""); // XXX HACK AWW!
        });

        add_method("empty", []() {
            // If there is no data, Debtags should work as an empty collection
            EnvOverride eo("DEBTAGS_TAGS", "./empty/notags");
            Debtags empty;

            wassert(actual(empty.begin() == empty.end()).istrue());
            wassert(actual(empty.timestamp()) == 0);
            wassert(actual(empty.hasData()).isfalse());

            set<std::string> res = empty.getTagsOfItem("apt");
            wassert(actual(res.empty()).istrue());

            res = empty.getAllTags();
            wassert(actual(res.empty()).istrue());
        });
    }
} tests("debtags");

}
