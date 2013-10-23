/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2013 by Bryan Jurish <moocow@cpan.org> and Kay-Michael Würzner <wuerzner@bbaw.de>

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
 * File: waste_main.cc
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot tagger: waste tokenizer: encoder
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

#include <wasteScanner.h>
#include <wasteTypes.h>
#include <mootUtils.h>
#include <mootCIO.h>
#include <mootToken.h>
#include <mootTokenIO.h>

#include "waste_cmdparser.h"

using namespace std;
using namespace moot;
using namespace mootio;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
const char *PROGNAME = "waste";
int vlevel;

//-- files
mofstream out;

//-- Token I/O: reader (for -no-scan)
int ifmt = tiofNone;
int ifmt_implied = tiofRare;
int ifmt_default = tiofRare|tiofLocation;
TokenReader *reader = NULL;

//-- Token I/O: writer
int ofmt = tiofNone;
int ofmt_implied = tiofNone;
int ofmt_default = tiofMediumRare|tiofLocation;
TokenWriter *writer = NULL;

//-- options & file-churning
gengetopt_args_info args;
cmdutil_file_churner churner;

//-- stats
int nfiles = 0;
int ntokens = 0;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- verbosity
  vlevel = args.verbose_arg;

  //-- operation mode
  if (!args.scan_flag && !args.lex_flag)
    moot_croak("%s: ERROR: you must enable at least one of -scan or -lex !\n", PROGNAME);

  // -- show banner
  if (!args.no_banner_given)
    moot_msg(vlevel, vlInfo, moot_program_banner(PROGNAME, PACKAGE_VERSION, "Bryan Jurish <moocow@cpan.org> and Kay-Michael Würzner").c_str());

  // -- output file
  if (!out.open(args.output_arg,"w"))
    moot_croak("%s: open failed for output-file \"%s\": %s\n", PROGNAME, out.name.c_str(), strerror(errno));

  // -- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = args.list_given;
  churner.paranoid = !args.recover_given;

  //-- i/o format : input
  if (!args.scan_flag)
    ifmt = TokenIO::parse_format_request(args.input_format_arg,
					 (args.inputs_num>0 ? args.inputs[0] : NULL),
					 ifmt_implied,
					 ifmt_default);

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
 * guts: -scan -lex
 */
void scan_lex_main(void) {
  moot_croak("%s: -scan -lex mode not yet implemented!\n", PROGNAME);
}

/*--------------------------------------------------------------------------
 * guts: -no-scan -lex
 */
void lex_main(void) {
  moot_croak("%s: -no-scan -lex mode not yet implemented!\n", PROGNAME);
}

/*--------------------------------------------------------------------------
 * guts: -scan -no-lex
 */
void scan_main(void) {
  wasteScanner scanner;

  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    if (vlevel >= vlInfo) ++nfiles;
    moot_msg(vlevel, vlProgress,  "%s: processing file '%s'... ", PROGNAME, churner.in.name.c_str());
    writer->printf_comment(" %s:File: %s\n", PROGNAME, churner.in.name.c_str());

    scanner.from_mstream(&churner.in);
    scanner.to_mstream(&out);
    int typ;
    mootToken tok;

    while ( (typ=scanner.yylex()) != 0 ) {
      tok.clear();
      switch ( typ ) {
      case wst_TOKEN_NL:
	tok.text("\\n");
	break;
      default:
	tok.text(scanner.yytext());
	break;
      }

      tok.insert( wasteScannerTypeNames[typ], "" );
      tok.location( scanner.theByte-scanner.yyleng(), scanner.yyleng() );
      writer->put_token(tok);
      ++ntokens;
    }
    writer->printf_comment("$EOF\t%lu 0\tEOF\n", scanner.theByte);
  }
}

/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GetMyOptions(argc,argv);

  if      (args.scan_flag && args.lex_flag) {
    scan_lex_main();
  }
  else if (!args.scan_flag && args.lex_flag) {
    lex_main();
  }
  else if (args.scan_flag && !args.lex_flag) {
    scan_main();
  }
  else {
    moot_croak("%s: ERROR: can't handle scan_flag=%d && lex_flag=%d combination!\n", PROGNAME, args.scan_flag, args.lex_flag);
  }

  moot_msg(vlevel, vlProgress, " done.\n");
  writer->close();
  out.close();

  // -- summary
  if (vlevel >= vlInfo) {
    double elapsed = static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC);

    // -- print summary
    fprintf(stderr, "\n-----------------------------------------------------\n");
    fprintf(stderr, "%s Summary:\n", PROGNAME);
    fprintf(stderr, "  + Files processed : %d\n", nfiles);
    fprintf(stderr, "  + Tokens found    : %d\n", ntokens);
    fprintf(stderr, "  + Time Elsapsed   : %.2f sec\n", elapsed);
    fprintf(stderr, "  + Throughput      : %.2f toks/sec\n", static_cast<double>(ntokens)/elapsed);
    fprintf(stderr, "-----------------------------------------------------\n");
  }
  
  return 0;
}

