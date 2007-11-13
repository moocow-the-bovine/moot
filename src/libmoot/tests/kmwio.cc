#include <mootIO.h>
#include <mootTokenIO.h>
#include <string>

// -- Common
static mootio::mistream* mis;       ///< current input stream.
static mootio::mostream* mos;       ///< current output stream
static moot::TokenReader* reader;   ///< current token reader  -- BUG: never allocated!
static moot::TokenWriter* writer;   ///< current token writer  -- BUG: never allocated!

int main (  )
{
	std::string ifn = "test.txt";
        std::string ofn = "test.out";
        //std::string ofn = (ifn == "-" || args.stdout_flag) ? "-" : dwds::replace_extension(ifn, args.osuffix_arg);
    
        //mis = dwds::open_istream(ifn);
        //mos = dwds::open_ostream(ofn);
	mis = new mootio::mfstream(ifn.c_str(), "r");
	mos = new mootio::mfstream(ofn.c_str(), "w");
    
        reader->from_mstream(mis);
        writer->to_mstream(mos);

	return 0;
}
