/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2002-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootpp_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's tagger: preprocessor : main()
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <mootPPLexer.h>
#include <mootUtils.h>
#include <mootCIO.h>
#include <mootToken.h>
#include <mootTokenIO.h>

#include "mootpp_cmdparser.h"

using namespace std;
using namespace moot;
using namespace mootio;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "mootpp";

//-- files
mofstream out;

//-- Token I/O
int ofmt = tiofNone;
int ofmt_implied = tiofNone;
int ofmt_default = tiofRare;
TokenWriter *writer = NULL;

//-- options & file-churning
gengetopt_args_info args;
cmdutil_file_churner churner;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  // -- show banner
  if (args.verbose_arg > 0)
    fprintf(stderr,
	    moot_program_banner(PROGNAME,
				PACKAGE_VERSION,
				"Bryan Jurish <jurish@ling.uni-potsdam.de>").c_str());


  // -- output file
  if (!out.open(args.output_arg,"w")) {
    fprintf(stderr,"%s: open failed for output-file \"%s\": %s\n",
	    PROGNAME, out.name.c_str(), strerror(errno));
    exit(1);
  }

  // -- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = args.list_given;

  //-- i/o format : output
  ofmt = TokenIO::parse_format_request(args.output_format_arg,
				       args.output_arg,
				       ofmt_implied,
				       ofmt_default);

  //-- setup token-writer
  writer = TokenIO::new_writer(ofmt);
  writer->to_mstream(&out);
}

/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  mootPPLexer *lexer = new mootPPLexer();
  int nfiles = 0;
  timeval started,stopped;
  double elapsed;

  GetMyOptions(argc,argv);
  lexer->verbose = args.verbose_arg;

  // -- get start time
  if (args.verbose_arg > 0) {
      gettimeofday(&started, NULL);
  }

  // -- big loop
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    if (args.verbose_arg > 0) {
      nfiles++;
      if (args.verbose_arg > 1) {
	fprintf(stderr,"%s: processing file '%s'... ",
		PROGNAME, churner.in.name.c_str());
	fflush(stderr);
      }
    }
    writer->printf_comment("\n    File: %s\n", churner.in.name.c_str());

    lexer->from_mstream(&churner.in);
    lexer->to_mstream(&out);

    int lxtok;
    mootSentence sent;
    while ((lxtok = lexer->yylex()) != mootPPLexer::PPEOF) {
      if (args.verbose_arg > 0) lexer->ntokens++;
      switch (lxtok) { 
      case mootPPLexer::EOS:
	if (lexer->yytext[0] != '<') {  //-- hack: check for xml markup
	  sent.push_back(mootToken((const char *)lexer->yytext));
	  writer->put_sentence(sent);
	  sent.clear();
	}
	break;

      case mootPPLexer::XML_START_TAG:
      case mootPPLexer::XML_END_TAG:
	//-- ignore XML tags
	break;

      default:
	//-- write it as its own token
	sent.push_back(mootToken((const char *)lexer->yytext));
	break;
      }
    }
    if (!sent.empty()) writer->put_sentence(sent); //-- put final sentence

    if (args.verbose_arg > 1) {
      fprintf(stderr," done.\n");
      fflush(stderr);
    }
  }
  writer->close();
  out.close();

  // -- summary
  if (args.verbose_arg > 0) {
    // -- timing
    gettimeofday(&stopped, NULL);
    elapsed = stopped.tv_sec-started.tv_sec + (stopped.tv_usec-started.tv_usec)/1000000.0;
    
    // -- print summary
    fprintf(stderr, "\n-----------------------------------------------------\n");
    fprintf(stderr, "%s Summary:\n", PROGNAME);
    fprintf(stderr, "  + Files processed : %d\n", nfiles);
    fprintf(stderr, "  + Tokens found    : %d\n", lexer->ntokens);
    fprintf(stderr, "  + Time Elsapsed   : %.2f sec\n", elapsed);
    fprintf(stderr, "  + Throughput      : %.2f toks/sec\n", (float)lexer->ntokens/elapsed);
    fprintf(stderr, "-----------------------------------------------------\n");
  }
  
  return 0;
}

