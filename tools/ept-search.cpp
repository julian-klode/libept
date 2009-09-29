#include <ept/apt/apt.h>
#include <ept/textsearch/textsearch.h>

using namespace ept;

struct Main {
    apt::Apt apt;
    textsearch::TextSearch ts;

    std::vector< std::string > args;

    Main( int argc, char **argv )
    {
        for ( int i = 1; i < argc; ++i )
            args.push_back( argv[i] );
    }

    int run() {
        using namespace std;
        Xapian::Enquire enq( ts.db() );
        enq.set_query( ts.makeORQuery( args.begin(), args.end() ) );

	// Limit to 20 matches
	Xapian::MSet matches = enq.get_mset(0, 20);
	for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); ++i)
	{
		// Filter out results that apt doesn't know
		if (!apt.isValid(i.get_document().get_data()))
		{
			cerr << "  Not in apt database: " << i.get_document().get_data().c_str() << endl;
			continue;
		}
		
		cout << apt.rawRecord(i.get_document().get_data()) << endl;
	}

        return 0;
    }
};

int main( int argc, char **argv ) {
    return Main( argc, argv ).run();
}
