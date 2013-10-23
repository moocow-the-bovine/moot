#include <mootCIO.h>
#include <wasteCase.h>

//======================================================================
int main ( int argc, char* argv[] )
{
  const char *prog = argv[0], *infile="-", *outfile="-";
  mootio::mistream *min = NULL;
  mootio::mostream *mout = NULL;

  //-- command-line: help
  if (argc > 1 && (strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--help")==0)) {
    fprintf(stderr, "Usage: %s [INFILE [OUTFILE]]\n", prog);
    exit(0);
  }

  //-- command-line: input file
  if (argc > 1)
    infile = argv[1];
  min = new mootio::mfstream(infile,"r");
  if (!min || !min->valid()) {
    fprintf(stderr, "%s: open failed for input file `%s': %s\n", prog, infile, strerror(errno));
    exit(1);
  }

  //-- command-line: output file
  if (argc > 2)
    outfile = argv[2];
  mout = new mootio::mfstream(outfile,"w");
  if (!mout || !mout->valid()) {
    fprintf(stderr, "%s: open failed for output file `%s': %s\n", prog, outfile, strerror(errno));
    exit(1);
  }

  //-- ye olde guttes
  mootio::ByteCount nbytes;
  std::string s,ls;
  for (nbytes=min->getline(s); nbytes >= 0 && !min->eof(); nbytes=min->getline(s)) {
    ls = moot::utf8ToLower(s);
    mout->puts(ls);
  }

  //-- cleanup
  mout->flush();
  if (min) delete min;
  if (mout) delete mout;

  return 0;
}
