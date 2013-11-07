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

#include <wasteTypes.h>
#include <wasteScanner.h>
#include <wasteLexer.h>
#include <wasteDecoder.h>

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

//-- Token I/O: reader (for --no-scan)
int ifmt = tiofNone;
int ifmt_implied = tiofNone;
int ifmt_default = tiofRare|tiofLocation;

//-- Token I/O: writer
int ofmt = tiofNone;
int ofmt_implied = tiofNone;
int ofmt_default = tiofMediumRare|tiofLocation;
TokenWriter *main_writer = NULL;

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
  if (!args.scan_flag && !args.lex_flag && !args.decode_flag)
    moot_croak("%s: ERROR: you must enable at least one of --scan, --lex, or --decode !\n", PROGNAME);

  if (args.decode_flag)
    args.scan_flag = args.lex_flag = 0;

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
  if (args.decode_flag) {
    ifmt_implied |= tiofTagged|tiofAnalyzed;
    ofmt_implied |= tiofText;
    ofmt_default |= tiofText|tiofTagged;
  }
  if (args.decode_flag || !args.scan_flag)
    ifmt = TokenIO::parse_format_request(args.input_format_arg,
					 (args.inputs_num>0 ? args.inputs[0] : NULL),
					 ifmt_implied,
					 ifmt_default);

  //-- i/o format : output
  ofmt = TokenIO::parse_format_request(args.output_format_arg,
				       args.output_arg,
				       ofmt_implied,
				       ofmt_default);

  //-- setup final token-writer
  main_writer = TokenIO::new_writer(ofmt);
  main_writer->to_mstream(&out);
}

/*--------------------------------------------------------------------------
 * guts: scanner/lexer: churn input from a TokenReader
 */
void churn_io(TokenReader *reader, TokenWriter *writer=main_writer)
{
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    ++nfiles;
    if (vlevel >= vlInfo) {
      moot_msg(vlevel, vlProgress,  "%s: processing file '%s'... ", PROGNAME, churner.in.name.c_str());
      writer->printf_comment(" %s:File: %s\n", PROGNAME, churner.in.name.c_str());
    }

    reader->from_mstream(&churner.in);
    mootTokenType toktyp;
    while ( (toktyp=reader->get_token()) != TokTypeEOF ) {
      writer->put_token( *(reader->token()) );
      ++ntokens;
    }
    if (vlevel >= vlInfo)
      writer->printf_comment("$EOF\t%lu 0\tEOF\n", reader->byte_number() );
  }
}


/*--------------------------------------------------------------------------
 * guts: scanner/lexer: setup lexer
 */
wasteLexer *get_lexer(int lexfmt=tiofUnknown, TokenReader *reader=NULL)
{
  wasteLexer *lexer = new wasteLexer(lexfmt);

  lexer->dehyph_mode(args.norm_hyph_flag > 0);

  if (args.abbrevs_given && !lexer->wl_abbrevs.load(args.abbrevs_arg))
    moot_croak("%s: ERROR: can't load abbreviation lexicon from '%s': %s", PROGNAME, args.abbrevs_arg, strerror(errno));

  if (args.conjunctions_given && !lexer->wl_conjunctions.load(args.conjunctions_arg))
    moot_croak("%s: ERROR: can't load conjunction lexicon from '%s': %s", PROGNAME, args.conjunctions_arg, strerror(errno));

  if (args.stopwords_given && !lexer->wl_stopwords.load(args.stopwords_arg))
    moot_croak("%s: ERROR: can't load stopword lexicon from '%s': %s", PROGNAME, args.stopwords_arg, strerror(errno));

  if (reader)
    lexer->from_reader(reader);

  return lexer;
}

/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GetMyOptions(argc,argv);

  if (args.scan_flag && args.lex_flag) {
    // --scan --lex
    wasteTokenScanner scanner( ofmt&(tiofText|tiofLocation|tiofTagged) );
    wasteLexer *lexer = get_lexer( ofmt, &scanner );
    churn_io( lexer );
    delete lexer;
  }
  else if (!args.scan_flag && args.lex_flag) {
    // --no-scan --lex
    TokenReader *reader = TokenIO::new_reader(ifmt);
    wasteLexer *lexer = get_lexer( ofmt, reader );
    churn_io( lexer );
    delete lexer;
  }
  else if (args.scan_flag && !args.lex_flag) {
    // --scan --no-lex
    wasteTokenScanner scanner(ofmt);
    churn_io( &scanner );
  }
  else if (args.decode_flag) {
    // --decode
    TokenReader  *reader  = TokenIO::new_reader(ifmt);
    wasteDecoder *decoder = new wasteDecoder();
    decoder->to_writer(main_writer);
    churn_io( reader, decoder );
    delete decoder;
    delete reader;
  }
  else {
    moot_croak("%s: ERROR: can't handle scan_flag=%d && lex_flag=%d && decode_flag=%d combination!\n", PROGNAME, args.scan_flag, args.lex_flag, args.decode_flag);
  }

  moot_msg(vlevel, vlProgress, " done.\n");
  main_writer->close();
  out.close();

  // -- summary
  if (vlevel >= vlInfo) {
    double elapsed = static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC);

    // -- print summary
    fprintf(stderr, "\n-----------------------------------------------------\n");
    fprintf(stderr, "%s Summary:\n", PROGNAME);
    if (!args.scan_flag)
      fprintf(stderr, "  + Input format    : \"%s\"\n", TokenIO::format_canonical_string(ifmt).c_str());
    fprintf(stderr, "  + Output format   : \"%s\"\n", TokenIO::format_canonical_string(ofmt).c_str());
    fprintf(stderr, "  + Files processed : %d\n", nfiles);
    fprintf(stderr, "  + Tokens found    : %d\n", ntokens);
    fprintf(stderr, "  + Time Elsapsed   : %.2f sec\n", elapsed);
    fprintf(stderr, "  + Throughput      : %.2f toks/sec\n", static_cast<double>(ntokens)/elapsed);
    fprintf(stderr, "-----------------------------------------------------\n");
  }
  
  return 0;
}

