/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2002-2014 Jurish <moocow@cpan.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootpp_main.cc
 * Author: Bryan Jurish <moocow@cpan.org>
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
const char *PROGNAME = "mootpp";
int vlevel;

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

  //-- verbosity
  vlevel = args.verbose_arg;

  // -- show banner
  if (!args.no_banner_given)
    moot_msg(vlevel, vlInfo, moot_program_banner(PROGNAME, PACKAGE_VERSION, "Bryan Jurish <moocow@cpan.org>").c_str());

  // -- output file
  if (!out.open(args.output_arg,"w"))
    moot_croak("%s: open failed for output-file \"%s\": %s\n", PROGNAME, out.name.c_str(), strerror(errno));

  // -- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = args.list_given;
  churner.paranoid = !args.recover_given;

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

  GetMyOptions(argc,argv);
  lexer->verbose = (vlevel >= vlInfo);

  // -- big loop
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    if (vlevel >= vlInfo) ++nfiles;
    moot_msg(vlevel, vlProgress,  "%s: processing file '%s'... ", PROGNAME, churner.in.name.c_str());
    writer->printf_comment("\n    File: %s\n", churner.in.name.c_str());

    lexer->from_mstream(&churner.in);
    lexer->to_mstream(&out);

    int lxtok;
    mootSentence sent;
    while ((lxtok = lexer->yylex()) != mootPPLexer::PPEOF) {
      if (vlevel >= vlInfo) ++lexer->ntokens;
      switch (lxtok) { 
      case mootPPLexer::EOS:
	if (lexer->yytext[0] != '<') {  //-- hack: check for xml markup
	  sent.push_back(mootToken(reinterpret_cast<const char*>(lexer->yytext)));
	  sent.back().location(mootToken::Location(lexer->theByte-lexer->yyleng,lexer->yyleng));
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
	sent.push_back(mootToken(reinterpret_cast<const char *>(lexer->yytext)));
	sent.back().location(mootToken::Location(lexer->theByte-lexer->yyleng,lexer->yyleng));
	break;
      }
    }
    if (!sent.empty()) writer->put_sentence(sent); //-- put final sentence

    moot_msg(vlevel, vlProgress, " done.\n");
  }
  writer->close();
  out.close();

  // -- summary
  if (vlevel >= vlInfo) {
    double elapsed = static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC);

    // -- print summary
    fprintf(stderr, "\n-----------------------------------------------------\n");
    fprintf(stderr, "%s Summary:\n", PROGNAME);
    fprintf(stderr, "  + Files processed : %d\n", nfiles);
    fprintf(stderr, "  + Tokens found    : %d\n", lexer->ntokens);
    fprintf(stderr, "  + Time Elsapsed   : %.2f sec\n", elapsed);
    fprintf(stderr, "  + Throughput      : %.2f toks/sec\n", static_cast<double>(lexer->ntokens)/elapsed);
    fprintf(stderr, "-----------------------------------------------------\n");
  }
  
  return 0;
}

