#include <stdio.h>

#include "mootConfig.h"
#include "mootToken.h"
#include "mootTokenExpatIO.h"

#include "dotest.h"
const bool checkMemory = false;

using namespace moot;

void churntest(int argc, char **argv) {
  FILE *infile = stdin;
  for ( ; argc > 0 ; argc--, argv++) {
    //-- open
    if (*argv) {
      infile = fopen(*argv, "r");
      if (!infile) {
	fprintf(stderr, "open failed for '%s'\n", *argv);
	exit(1);
      }
    } else {
      infile = stdin;
    }

    //-- init
    int tr_format = (tiofXML
		     | tiofWellDone
		     );
    TokenReaderExpat tr(tr_format);
    //tr.save_raw_xml = true;
    tr.save_raw_xml = false;
    
    DOTEST_START ;

    //-- guts
    tr.from_file(infile);
    int lxtyp;
    size_t ntokens = 0;
    for (lxtyp = tr.get_token(); lxtyp != TokTypeEOF; lxtyp = tr.get_token()) {
      mootToken *tok = tr.token();

      printf("TOKEN: ");
      if (!tok) { printf("(null)\n"); continue; }

      ntokens++;
      printf("type=`%s' ; text=`%s' ; best=`%s'\n",
	     mootTokenTypeNames[tok->toktype()],
	     tok->text().c_str(),
	     tok->besttag().c_str());
      
      for (mootToken::Analyses::const_iterator ai = tok->analyses().begin();
	   ai != tok->analyses().end();
	   ai++)
	{
	  printf("\t  - ANALYSIS: tag=`%s'\t ; details=`%s'\t\n",
		 ai->tag.c_str(),
		 ai->details.c_str());
	}
    }
    printf("%%%%--EOF--\n");

    DOTEST_STOP ;
    dotest_report((*argv ? *argv : "-"), ntokens);

    if (infile != stdin) fclose(infile);

    //-- check memory
    if (checkMemory) {
      fprintf(stderr, "Check memory usage ? ");
      fgetc(stdin);
    }
  }
}

int main (int argc, char **argv) {
  char *default_argv[1] = {NULL};

  fprintf(stderr, "%s (%s v%s)\n", *argv, PACKAGE_NAME, PACKAGE_VERSION);

  if (checkMemory) {
    fprintf(stderr, "Check memory usage ? "); fgetc(stdin);
  }

  churntest((argc > 1 ? argc-1 : 1),
	    (argc > 1 ? argv+1 : default_argv));

  return 0;
}
