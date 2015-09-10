#include "ept/test.h"
#include "vocabulary.h"
#include "coll/set.h"
#include "ept/test.h"

using namespace std;
using namespace ept::debtags::coll::utils;
using namespace ept::debtags;
using namespace ept::tests;

#define testfile "debtags/vocabulary"

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("load", []() {
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary  tags; // this will throw if it failed to load
        });

        add_method("has_facet", []() {
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary tags;
            assert_true( tags.hasFacet( "works-with" ) );
            assert_true( !tags.hasFacet( "blah" ) );
        });

        add_method("has_tag", []() {
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary tags;
            assert_true( tags.hasTag( "works-with::people" ) );
            assert_true( !tags.hasTag( "works-with::foobar" ) );
        });

        add_method("tagdata", []() {
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary tags;
            const voc::TagData *people = tags.tagData( "works-with::people" ),
                               *foobar = tags.tagData( "works-with::foobar" ),
                               *blahg = tags.tagData( "works-with::blahg" ),
                               *text = tags.tagData( "works-with::text" ),
                               *people2 = tags.tagData( "works-with::people" );
            assert_true( people != foobar );
            assert_true( people != text );
            assert_true( people != blahg );
            assert_true( foobar == blahg );
            assert_true( foobar == foobar );
            assert_true( people == people2 );
            assert_true( people == people );
        });

        add_method("tags", []() {
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary tags;
            std::string a = "works-with::people",
                        b = "works-with::foobar";
            std::set<std::string> s = tags.tags(),
                                  f = tags.tags( "works-with" ),
                                  n = tags.tags( "nonsense" );
            assert_true( set_contains(s, a) );
            assert_true( set_contains(f, a) );
            assert_true( set_contains(s, f) );
            assert_true( !set_contains(s, b) );
            assert_true( !set_contains(f, b) );
            assert_true( n.empty() );
        });

        add_method("facetdata", []() {
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary tags;

            const voc::FacetData* f = tags.facetData( "works-with" );
            assert_true(f);
            wassert(actual(f->name) == "works-with");

            const voc::TagData* t = tags.tagData( "works-with::people" );
            assert_true(t);
            wassert(actual(t->name) == "works-with::people");
        });

        add_method("facettags", []() {
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary tags;

            const voc::FacetData* f = tags.facetData( "works-with" );
            std::set<std::string> x = tags.tags( "works-with" );
            assert_true( x == f->tags() );
        });

        add_method("missing_facet", []() {
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary tags;

            const voc::FacetData* f = tags.facetData( "does-not-work-with" );
            assert_true(!f);
        });

        add_method("missing_facet1", []() {
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary tags;

            const voc::FacetData* f = tags.facetData( "legacy" );
            assert_true(f);
            wassert(actual(f->shortDescription()) == "");
            wassert(actual(f->longDescription()) == "");
            //wassert(actual(f.shortDescription( "weehee" )) == "weehee");
        });

        add_method("one_letter_tag", []() {
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary tags;

            // assert_true that one-character tag names are parsed correctly
            assert_true( tags.hasTag( "implemented-in::c" ) );
        });

        add_method("iterate_facets", []() {
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary tags;

            // assert_true that all facets are somehow working
            std::set<std::string> facets = tags.facets();

            for (std::set<std::string>::const_iterator i = facets.begin();
                    i != facets.end(); i++)
            {
                const voc::FacetData* f = tags.facetData(*i);
                assert_true(f);
            }
        });

        add_method("iterate_tags", []() {
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary voc;

            // assert_true that all tags are somehow working
            std::set<std::string> tags = voc.tags();
            for (std::set<std::string>::const_iterator i = tags.begin();
                    i != tags.end(); i++)
            {
                const voc::TagData* t = voc.tagData(*i);
                assert_true(t);
            }
        });

        add_method("first_last", []() {
            // Check for correctness of the first and last tag in the vocabulary
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary tags;

            const voc::TagData* first = tags.tagData("accessibility::TODO");
            assert_true(first);
            wassert(actual(first->name) == string("accessibility::TODO"));
            wassert(actual(first->shortDescription()) == string("Need an extra tag"));

            const voc::TagData* last = tags.tagData("x11::xserver");
            assert_true(last);
            wassert(actual(last->name) == string("x11::xserver"));
            wassert(actual(last->shortDescription()) == string("X Server"));
        });

        add_method("get_all_tags", []() {
            EnvOverride eo("DEBTAGS_VOCABULARY", testfile);
            Vocabulary tags;

            // check that we're seeing all the tags for a facet
            std::set<std::string> t = tags.tags("accessibility");
            wassert(actual(t.size()) == 10u);

            t = tags.tags("works-with-format");
            wassert(actual(t.size()) == 33u);
        });

        add_method("empty", []() {
            // If there is no data, Vocabulary should work as an empty vocabulary
            EnvOverride eo("DEBTAGS_VOCABULARY", "./empty/novocabularyhere");
            Vocabulary empty;

            assert_true(!empty.hasData());

            set<std::string> facets = empty.facets();
            wassert(actual(facets.size()) == 0u);

            set<std::string> tags = empty.tags();
            wassert(actual(tags.size()) == 0u);
        });
    }
} tests("debtags_vocabulary");

}
