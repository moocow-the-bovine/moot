//-*- Mode: C++; coding: utf-8; c-basic-offset: 2; -*-
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
#include <wasteTrainWriter.h>

#include <mootUtils.h>
#include <mootCIO.h>
#include <mootToken.h>
#include <mootTokenIO.h>
#include <mootHMM.h>

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

//-- optional objects
TokenReader  *reader=NULL;
wasteTokenScanner *scanner=NULL;
wasteLexer   *lexer=NULL;
mootHMM      *tagger=NULL;
wasteDecoder *decoder=NULL;
wasteTrainWriter *trainwriter=NULL;

//-- Token I/O: reader; see GetMyOptions() for mode-dependent hacks
int ifmt = tiofNone;
int ifmt_implied = tiofNone;
int ifmt_default = tiofText|tiofLocation;

//-- Token I/O: writer; ; see GetMyOptions() for mode-dependent hacks
int ofmt = tiofNone;
int ofmt_implied = tiofText;
int ofmt_default = tiofText|tiofLocation; 
TokenWriter *main_writer = NULL;

//-- options & file-churning
gengetopt_args_info args;
cmdutil_file_churner churner;

//-- stats
int nfiles = 0;
int ntokens = 0;

//-- mode bitmask flags
const int wmNone   = 0;
const int wmScan   = 0x01;
const int wmLex    = 0x02;
const int wmTag    = 0x04;
const int wmDecode = 0x08;
const int wmTrain  = 0x10;

const int wmFull = wmScan|wmLex|wmTag|wmDecode;
int mode = wmNone;
std::string modestr("none");


/*--------------------------------------------------------------------------
 * Mode Utils
 */
inline int& modeSet(int &m, int flag, bool enabled) {
  if (enabled) {
    m |= flag;
  }
  else {
    m &= (~flag);
  }
  return m;
}

inline int modeFirst(int m) {
  if      (m&wmScan) return wmScan;
  else if (m&wmLex)  return wmLex;
  else if (m&wmTag)  return wmTag;
  else if (m&wmDecode) return wmDecode;
  else if (m&wmTrain) return wmTrain;
  return wmNone;
}
inline int modeLast(int m) {
  if      (m&wmTrain) return wmTrain;
  else if (m&wmDecode) return wmDecode;
  else if (m&wmTag)  return wmTag;
  else if (m&wmLex)  return wmLex;
  else if (m&wmScan) return wmScan;
  return wmNone;
}
inline std::string modeString(int m) {
  std::string s("");
  if (m&wmScan) s += "Scan,";
  if (m&wmLex) s += "Lex,";
  if (m&wmTag) s += "Tag,";
  if (m&wmDecode) s += "Decode,";
  if (m&wmTrain) s += "Train,";
  if (s.empty()) { s = "None"; }
  else { s.erase(s.size()-1); }
  return s;
}


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
  if ( !args.train_given && !args.full_given && !args.scan_given && !args.lex_given && !args.tag_given && !args.decode_given ) {
    args.full_flag = args.scan_flag = args.lex_flag = args.tag_flag = args.decode_flag = 1;
  }
  modeSet(mode, wmScan, args.scan_flag);
  modeSet(mode, wmLex,  args.lex_flag);
  modeSet(mode, wmTag,  args.tag_flag);
  modeSet(mode, wmDecode, args.decode_flag);
  modeSet(mode, wmTrain,  args.train_flag);
  modestr = modeString(mode);
  //fprintf(stderr, "%s: DEBUG: mode=%s\n", PROGNAME, modestr.c_str());

  //-- sanity check(s)
  if ( mode==wmNone ) {
    moot_croak("%s: ERROR: you must enable at least one of --scan, --lex, --tag, --decode, or --train\n", PROGNAME);
  }
  else if ( (mode&wmTrain) && (mode&(~wmTrain)) ) {
    moot_croak("%s: ERROR: cannot combine --train with any other operation mode\n", PROGNAME);
  }
  else if ( (mode&wmScan) && !(mode&wmLex) && (mode&(wmTag|wmDecode)) )  {
    moot_croak("%s: ERROR: cannot combine --scan and --decode/--tag without --lex\n", PROGNAME);
  }
  else if ( (mode&(wmScan|wmLex)) && !(mode&wmTag) && (mode&wmDecode) ) {
    moot_croak("%s: ERROR: cannot combine --scan/--lex and --decode without --tag\n", PROGNAME);
  }
  else if ( (mode&wmTag) && !args.model_given ) {
    moot_croak("%s: ERROR: --tag mode requires --model option\n", PROGNAME);
  }

  //-- show banner
  if (!args.no_banner_given)
    moot_msg(vlevel, vlInfo, moot_program_banner(PROGNAME, PACKAGE_VERSION, "Bryan Jurish <moocow@cpan.org> and Kay-Michael Würzner").c_str());

  //-- output file
  if (!out.open(args.output_arg,"w"))
    moot_croak("%s: open failed for output-file \"%s\": %s\n", PROGNAME, out.name.c_str(), strerror(errno));

  //-- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = args.list_given;
  churner.paranoid = !args.recover_given;

  //-- i/o formats: mode-dependent hacks: input
  switch (modeFirst(mode)) {
  case wmScan:		break;
  case wmLex:		ifmt_implied |= tiofText; break;
  case wmTag:		ifmt_implied |= tiofText|tiofAnalyzed; break;
  case wmDecode:	ifmt_implied |= tiofText|tiofTagged|tiofAnalyzed; break;
  case wmTrain:		ifmt_implied |= tiofText; break;
  default: break;
  }

  //-- i/o formats: mode-dependent hacks: output
  switch (modeLast(mode)) {
  case wmTrain:		ofmt_implied |= tiofText|tiofTagged|tiofAnalyzed; break;
  case wmDecode:	ofmt_implied |= tiofTagged; break;
  case wmTag:		ofmt_implied |= tiofTagged; break;
  case wmLex:		ofmt_implied |= tiofAnalyzed; break;
  case wmScan:		break;
  default: break;
  }

  //-- i/o format : input
  if ( !(mode&wmScan) )
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
void churn_io(TokenReader *reader, TokenWriter *writer=main_writer, mootHMM *hmm=tagger)
{
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    ++nfiles;
    if (vlevel >= vlInfo) {
      moot_msg(vlevel, vlProgress,  "%s: processing file '%s'... ", PROGNAME, churner.in.name.c_str());
      writer->printf_comment(" %s:File: %s\n", PROGNAME, churner.in.name.c_str());
    }

    reader->from_mstream(&churner.in);
    if (hmm) {
      hmm->ntokens = 0;
      hmm->tag_stream(reader,writer);
      ntokens += hmm->ntokens;
    }
    else {
      ntokens += TokenIO::pipe_tokens( reader, writer );
    }
    if (vlevel >= vlInfo)
      writer->printf_comment("$EOF\t%lu 0\tEOF\n", reader->byte_number() );
  }
}

/*--------------------------------------------------------------------------
 * guts: scanner/lexer: setup lexer
 */
void setup_lexer(wasteLexer *lexer, TokenReader *reader=NULL)
{
  if (args.abbrevs_given && !lexer->wl_abbrevs.load(args.abbrevs_arg))
    moot_croak("%s: ERROR: can't load abbreviation lexicon from '%s': %s\n", PROGNAME, args.abbrevs_arg, strerror(errno));

  if (args.conjunctions_given && !lexer->wl_conjunctions.load(args.conjunctions_arg))
    moot_croak("%s: ERROR: can't load conjunction lexicon from '%s': %s\n", PROGNAME, args.conjunctions_arg, strerror(errno));

  if (args.stopwords_given && !lexer->wl_stopwords.load(args.stopwords_arg))
    moot_croak("%s: ERROR: can't load stopword lexicon from '%s': %s\n", PROGNAME, args.stopwords_arg, strerror(errno));

  if (reader)
    lexer->from_reader(reader);
}

//--------------------------------------------------------------------------
wasteLexer *get_lexer(int lexfmt=tiofUnknown, TokenReader *reader=NULL)
{
  wasteLexer *lexer = new wasteLexer(lexfmt);
  lexer->dehyph_mode(args.dehyphenate_flag > 0);
  setup_lexer(lexer,reader);
  return lexer;
}

/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GetMyOptions(argc,argv);
  TokenReader *churn_reader = NULL;
  TokenWriter *churn_writer = main_writer;

  //-- setup: globals
  if (mode&wmScan) {
    churn_reader = scanner = new wasteTokenScanner( ifmt|(ofmt&tiofLocation) );
    if (modeLast(mode)==wmScan)
      scanner->tr_format |= ofmt;
  } else {
    churn_reader = reader = TokenIO::new_reader(ifmt);
  }
  if (mode&wmLex) {
    lexer = get_lexer( ofmt, churn_reader );
    churn_reader = lexer;
  }
  if (mode&wmTag) {
    tagger = new mootHMM();
    tagger->verbose = args.verbose_arg;
    if (!tagger->load_model(args.model_arg))
      moot_croak("%s: ERROR: failed to load model from `%s'\n", PROGNAME, args.model_arg);
  }
  if (mode&wmDecode) {
    decoder = new wasteDecoder();
    decoder->to_writer(churn_writer);
    churn_writer = decoder;
  }
  if (mode&wmTrain) {
    trainwriter = new wasteTrainWriter( ofmt );
    setup_lexer( &trainwriter->wt_lexer, churn_reader );
    trainwriter->to_writer(churn_writer);
    churn_writer = trainwriter;
  }

  //-- churn
  churn_io(churn_reader, churn_writer, tagger);

  //-- cleanup
  if (trainwriter) delete trainwriter;
  if (decoder) delete decoder;
  if (tagger)  delete tagger;
  if (lexer)   delete lexer;
  if (scanner) delete scanner;
  if (reader)  delete reader;
  churn_reader = NULL;
  churn_writer = NULL;

  //-- finish
  moot_msg(vlevel, vlProgress, " done.\n");
  main_writer->close();
  out.close();

  // -- summary
  if (vlevel >= vlInfo) {
    double elapsed = static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC);

    // -- print summary
    fprintf(stderr, "\n-----------------------------------------------------\n");
    fprintf(stderr, "%s Summary:\n", PROGNAME);
    fprintf(stderr, "  + %-18s: \"%s\"\n", "Processing Mode", modestr.c_str());
    fprintf(stderr, "  + %-18s: \"%s\"\n", "Input format", TokenIO::format_canonical_string(ifmt).c_str());
    fprintf(stderr, "  + %-18s: \"%s\"\n", "Output format", TokenIO::format_canonical_string(ofmt).c_str());
    fprintf(stderr, "  + %-18s: %d\n", "File(s) processed", nfiles);
    fprintf(stderr, "  + %-18s: %d\n", "Token(s) found", ntokens);
    fprintf(stderr, "  + %-18s: %.2f sec\n", "Time elapsed", elapsed);
    fprintf(stderr, "  + %-18s: %.2f tok/sec\n", "Throughput", static_cast<double>(ntokens)/elapsed);
    fprintf(stderr, "-----------------------------------------------------\n");
  }
  
  return 0;
}

