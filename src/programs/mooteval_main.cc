/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mooteval_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's tagger: evaluator: main()
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <stdexcept>

#include <mootTokenIO.h>
#include <mootTokenExpatIO.h>
#include <mootEval.h>

#include <mootCIO.h>
#include <mootBufferIO.h>
#include <mootUtils.h>
#include "mooteval_cmdparser.h"

using namespace std;
using namespace moot;
using namespace mootio;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "mooteval";

// files
mifstream file1;
mifstream file2;
mofstream out;

// options & file-churning
gengetopt_args_info args;

//-- token i/o
int ifmt         = tiofNone;
int ifmt_implied = tiofNone;
int ifmt_default = tiofWellDone;

TokenReader *treader1 =NULL;
TokenReader *treader2 =NULL;

const int ofmt = tiofNative|tiofWellDone;

TokenWriterNative *twriter1 = new TokenWriterNative(ofmt);
TokenWriterNative *twriter2 = new TokenWriterNative(ofmt);
//mootSentence s1, s2;
mootTokenType tt1, tt2, tt1prev, tt2prev;

//-- comparison
mootEval eval;
//int status;

//-- information
size_t ntokens = 0;     //-- total number of tokens
size_t nsents = 0;      //-- total number of sentences
size_t ntokmisses = 0;  //-- number of token-text mismatches
size_t nbestmisses = 0; //-- number of best-tag mismatches

//-- information: file specific
size_t nempties1 = 0;   //-- number of empty analyses in file1
size_t nempties2 = 0;   //-- number of empty analyses in file2
size_t nimps1 = 0;      //-- number of impossible analyses in file1
size_t nimps2 = 0;      //-- number of impossible analyses in file2
size_t nximps1 = 0;     //-- number of x-impossible analyses in file1
size_t nximps2 = 0;     //-- number of x-impossible analyses in file2

unsigned long nans1 = 0; //-- total number of analyses in file1
unsigned long nans2 = 0; //-- total number of analyses in file2

size_t nsaves1 = 0;   //-- saves (empty class & x-best match) in file 1 vs. 2
size_t nsaves2 = 0;   //-- saves (empty class & x-best match) in file 2 vs. 1

size_t nfumbles1 = 0; //-- fumbles (+x-class-cover, -best-match) in file 1 vs. 2
size_t nfumbles2 = 0; //-- fumbles (+x-class-cover, -best-match) in file 2 vs. 1

size_t nharderrors =0;       //-- number of 'hard' (EOS/EOF) errors
size_t nharderrors_max =100; //-- maximum number of 'hard' errors

//-- output flags
bool eval_file1 = false;
bool eval_file2 = false;


//-- verbosity levels
typedef enum {
  evlSilent = 0,       //-- be silent (only print summary info)
  evlBasic = 10,       //-- print information + basic summary info
  evlExtended = 20,    //-- print information + extended summary info
  evlMistakes = 30,    //-- print tokens with errors
  evlEverything = 40   //-- print everything
} verbosityLevel;

int vlevel = 10; //-- our verbosity level

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- show banner
  if (args.verbose_arg > 0)
    fprintf(stderr,
	    moot_program_banner(PROGNAME,
				PACKAGE_VERSION,
				"Bryan Jurish <jurish@ling.uni-potsdam.de>").c_str());
  //-- options: verbosity
  if (args.verbose_arg <= 0)      vlevel = evlSilent;
  else if (args.verbose_arg <= 1) vlevel = evlBasic;
  else if (args.verbose_arg <= 2) vlevel = evlExtended;
  else if (args.verbose_arg <= 3) vlevel = evlMistakes;
  else /*if (args.verbose_arg > 3)*/ vlevel = evlEverything;

  //-- options: file selection
  if (args.eval_first_given || args.eval_second_given) {
    if (args.eval_first_given) eval_file1 = true;
    if (args.eval_second_given) eval_file2 = true;
  } else {
    eval_file1 = true;
    eval_file2 = true;
  }

  //-- options: output file
  if (!out.open(args.output_arg,"w")) {
    fprintf(stderr,"%s: open failed for output-file '%s': %s\n",
	    PROGNAME, out.name.c_str(), strerror(errno));
    exit(1);
  }
  twriter2->to_mstream(&out);

  //-- process arguments: input files
  if (args.inputs_num < 2) {
    fprintf(stderr, "%s: you must specify two files to compare!\n", PROGNAME);
    cmdline_parser_print_help();
    exit(1);
  }

  //-- i/o format: input
  ifmt = TokenIO::parse_format_request(args.input_format_arg,
				       (args.inputs_num>0 ? args.inputs[0] : NULL),
				       ifmt_implied,
				       ifmt_default);
  //-- io: new_reader
  treader1 = TokenIO::new_reader(ifmt);
  treader2 = TokenIO::new_reader(ifmt);

#ifdef MOOT_EXPAT_ENABLED
  //-- i/o format: input: xml: encoding
  if (ifmt&tiofXML && args.input_encoding_given) {
    ((TokenReaderExpat*)treader1)->setEncoding(args.input_encoding_arg);
    ((TokenReaderExpat*)treader2)->setEncoding(args.input_encoding_arg);
  }
#endif

  //-- open: file1
  if (!file1.open(args.inputs[0],"r")) {
    fprintf(stderr, "%s: open failed for file \"%s\": %s",
	    PROGNAME, file1.name.c_str(), strerror(errno));
    exit(2);
  }
  treader1->from_mstream(&file1);

  //-- open: file2
  if (!file2.open(args.inputs[1],"r")) {
    fprintf(stderr, "%s: open failed for file \"%s\": %s",
	    PROGNAME, file2.name.c_str(), strerror(errno));
    exit(2);
  }
  treader2->from_mstream(&file2);

  //-- process options
  //(none)
}

/*--------------------------------------------------------------------------
 * token output
 *--------------------------------------------------------------------------*/
void put_tok_results(FILE *file, const mootToken &tok1, const mootToken &tok2);

/*--------------------------------------------------------------------------
 * summary (new)
 *--------------------------------------------------------------------------*/
void print_summary(TokenWriter *tw);

void print_summary_for_file(TokenWriter *tw,
			    size_t ntokens, size_t nanls,
			    size_t nempties, size_t nimps, size_t nximps,
			    size_t nsaves, size_t nfumbles);

void print_summary_to_file(FILE *file);


/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  try{
    GetMyOptions(argc,argv);

    //-- Dummy token
    mootToken tokDummy("%% (EMPTY)");
    tokDummy.toktype(TokTypeVanilla); //-- really a comment

    mootToken tokEOS("%% (EOS)");
    tokDummy.toktype(TokTypeVanilla); //-- really a comment

    mootToken tokEOF("%% (EOF)");
    tokDummy.toktype(TokTypeVanilla); //-- really a comment

    //-- input configuration : ignore comments completely
    //treader1.lexer.ignore_comments = true;
    //treader2.lexer.ignore_comments = true;

    //-- compare files
    tt1prev = TokTypeEOS;
    tt2prev = TokTypeEOS;
    for (tt1  = treader1->get_token()  ,  tt2  = treader2->get_token();
	 tt1 != TokTypeEOF             || tt2 != TokTypeEOF;
	 /*treader1->get_token(), treader2->get_token()*/ )
      {
	//-- assign token aliases
	const mootToken *tok1 = (tt1 == TokTypeEOS
				 ? &tokEOS
				 : (tt1 == TokTypeEOF || !treader1->token()
				    ? &tokEOF
				    : treader1->token()));
	const mootToken *tok2 = (tt2 == TokTypeEOS
				 ? &tokEOS
				 : (tt2 == TokTypeEOF || !treader2->token()
				    ? &tokEOF
				    : treader2->token()));

	//-- check for too many 'hard' errors
	if (nharderrors > nharderrors_max) {
	  fprintf(stderr, "%s: too many hard errors (%u > %u) -- bailing out!\n",
		  PROGNAME, nharderrors, nharderrors_max);
	  break;
	}

	//-- check for comments
	if (tt1 == TokTypeComment || tt2 == TokTypeComment) {
	  eval.clear();
	  if (tt1==tt2) {
	    //-- comments in both files
	    put_tok_results(out.file, *tok1, *tok2);
	    //tt1prev = tt1;
	    //tt2prev = tt2;
	    tt1 = treader1->get_token();
	    tt2 = treader2->get_token();
	  }
	  else if (tt1==TokTypeComment) {
	    //-- comment in file1 only
	    put_tok_results(out.file, *tok1, *tok2);
	    //tt1prev = tt1;
	    tt1 = treader1->get_token();
	  }
	  else if (tt2==TokTypeComment) {
	    put_tok_results(out.file, *tok1, *tok2);
	    //tt2prev = tt2;
	    tt2 = treader2->get_token();
	  }
	  continue;
	}

	//-- check for EOS
	else if (tt1 == TokTypeEOS || tt2 == TokTypeEOS) {
	  if (tt1==tt2) {
	    //-- eos: match
	    nsents++;
	    if (vlevel >= evlEverything) fputc('\n', out.file);
	    tt1prev = tt1;
	    tt1 = treader1->get_token();
	    tt2prev = tt2;
	    tt2 = treader2->get_token();
	    continue;
	  }
	  else if (tt1prev==TokTypeEOS || tt2prev==TokTypeEOS) {
	    //-- ignore consecutive EOS tokens
	    if (tt1 == TokTypeEOS) {
	      tt1prev = tt1;
	      tt1 = treader1->get_token();
	    }
	    if (tt2 == TokTypeEOS) {
	      tt2prev = tt2;
	      tt2 = treader2->get_token();
	    }
	    continue;
	  }
	  //-- report real eos mismatch
	  fprintf(stderr, "%s: sentence lengths differ at lines %u/%u!\n",
		  PROGNAME, treader1->line_number(), treader2->line_number());
	  nharderrors++;

	  if (tt1 == TokTypeEOS) {
	    tt1prev = tt1;
	    tt1 = treader1->get_token();
	  }
	  if (tt2 == TokTypeEOS) {
	    tt2prev = tt2;
	    tt2 = treader2->get_token();
	  }
	  continue;
	}

	//-- check for EOF
	else if (tt1 == TokTypeEOF || tt2 == TokTypeEOF) {
	  if (tt1==tt2) break; //-- eof: match

	  else if (tt1 != TokTypeEOF) {
	    put_tok_results(out.file, *tok1, *tok2);
	    tt1prev = tt1;
	    tt1 = treader1->get_token();
	    if (tt1prev == TokTypeEOS) continue; //-- skip eof/eos mismatch
	  }
	  else if (tt2 != TokTypeEOF) {
	    put_tok_results(out.file, *tok1, *tok2);
	    tt2prev = tt2;
	    tt2 = treader2->get_token();
	    if (tt2prev == TokTypeEOS) continue; //-- skip eof/eos mismatch
	  }

	  //-- report real eof mismatch
	  fprintf(stderr, "%s: file lengths differ at lines %u/%u!\n",
		  PROGNAME, treader1->line_number(), treader2->line_number());
	  nharderrors++;
	  continue;
	}

	//--------------------------------------------------------
	//-- "plain" tokens: compare
	ntokens++;
	nans1 += tok1->analyses().size();
	nans2 += tok2->analyses().size();

	eval.compareTokens(*tok1,*tok2);
	if (eval.isTokenMismatch()) ntokmisses++;
	if (eval.isBestMismatch()) nbestmisses++;

	if (eval.isEmptyClass1()) {
	  nempties1++;
	  if (!eval.isBestMismatch()) nsaves1++;
	}
	else if (eval.isBestMismatch()) {
	  nfumbles1++;
	}
      
	if (eval.isEmptyClass2()) {
	  nempties2++;
	  if (!eval.isBestMismatch()) nsaves2++;
	}
	else if (eval.isBestMismatch()) {
	  nfumbles2++;
	}
      
	if (eval.isImpClass1()) nimps1++;
	if (eval.isImpClass2()) nimps2++;
      
	if (eval.isXImpClass1()) nximps1++;
	if (eval.isXImpClass2()) nximps2++;
      
	//-- output evaluation results
	put_tok_results(out.file, *tok1, *tok2);
      
	//-- get next token in each file
	tt1prev = tt1;
	tt2prev = tt2;
	tt1 = treader1->get_token();
	tt2 = treader2->get_token();
      }

    //-- summary
    print_summary(twriter2);

    if (vlevel >= evlBasic && out.file != stdout) {
      print_summary_to_file(stderr);
    }

    //-- cleanup
    treader1->close();
    treader2->close();

    twriter1->close();
    twriter2->close();

    file1.close();
    file2.close();
    out.close();
  }
  catch (exception &e) {
    fprintf(stderr, "%s: Exception: %s\n", PROGNAME, e.what());
    abort();
  }
  return 0;
}


/*--------------------------------------------------------------------------
 * summary (to FILE*)
 *--------------------------------------------------------------------------*/
void print_summary_to_file(FILE *file)
{
  TokenWriterNative twn(tiofWellDone);
  twn.to_file(file);
  print_summary(&twn);
  twn.close();
}

/*--------------------------------------------------------------------------
 * summary (new)
 *--------------------------------------------------------------------------*/
void print_summary(TokenWriter *tw)
{
  if (ntokens==0) ntokens = 1;
  tw->put_comment_block_begin();
  tw->printf_raw("===================================================================\n");
  tw->printf_raw(" %s Summary:\n", PROGNAME);

  //----------------------------------------------------
  // General
  tw->printf_raw("  + NTokens             : %9u\n", ntokens);
  tw->printf_raw("  + NSentences          : %9u\n", nsents);

  //----------------------------------------------------
  // File-1
  tw->printf_raw("  + File-1              : %s\n", file1.name.c_str());
  tw->printf_raw("    - Input format      : \"%s\"\n",
		 TokenIO::format_canonical_string(treader1->tr_format).c_str());
  if (vlevel >= evlExtended && eval_file1)
    print_summary_for_file(tw,
			   ntokens, nans1, nempties1,
			   nimps1, nximps1, nsaves1, nfumbles1);

  //----------------------------------------------------
  // File-2
  tw->printf_raw("  + File-2              : %s\n", file2.name.c_str());
  tw->printf_raw("    - Input format      : \"%s\"\n",
		 TokenIO::format_canonical_string(treader2->tr_format).c_str());
  if (vlevel >= evlExtended && eval_file2)
    print_summary_for_file(tw,
			   ntokens, nans2, nempties2,
			   nimps2, nximps2, nsaves2, nfumbles2);

  //----------------------------------------------------
  // Accuracy
  tw->printf_raw("  + Tokens Equal  (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  ntokens-ntokmisses, 100.0*(double)(ntokens-ntokmisses)/(double)ntokens,
	  ntokmisses, 100.0*(double)(ntokmisses)/(double)ntokens);

  tw->printf_raw("  + Tags Equal    (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  ntokens-nbestmisses, 100.0*(double)(ntokens-nbestmisses)/(double)ntokens,
	  nbestmisses, 100.0*(double)(nbestmisses)/(double)ntokens);

  tw->printf_raw("===================================================================\n");
  tw->put_comment_block_end();
}




/*--------------------------------------------------------------------------
 * summary (for file)
 *--------------------------------------------------------------------------*/
void print_summary_for_file(TokenWriter *tw,
			    size_t ntokens,
			    size_t nanls,
			    size_t nempties,
			    size_t nimps,
			    size_t nximps,
			    size_t nsaves,
			    size_t nfumbles)
{
  //----------------------------------------------------
  // File-N
  tw->printf_raw("    - Avg. Class Size   : %39.2f\n",
	  (double)nanls/(double)(ntokens-nempties)
	  );
  tw->printf_raw("    - Class Given (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  ntokens-nempties, 100.0*(double)(ntokens-nempties)/(double)ntokens,
	  nempties, 100.0*(double)nempties/(double)ntokens
	  );
  tw->printf_raw("    - Saves       (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  nsaves, 100.0*(double)nsaves/(double)nempties,
	  nempties-nsaves, 100.0*(double)(nempties-nsaves)/(double)nempties
	  );
  tw->printf_raw("    - Int. Cover  (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  ntokens-nimps, 100.0*(double)(ntokens-nimps)/(double)ntokens,
	  nimps, 100.0*(double)nimps/(double)ntokens
	  );
  tw->printf_raw("    - Ext. Cover  (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  ntokens-nximps, 100.0*(double)(ntokens-nximps)/(double)ntokens,
	  nximps, 100.0*(double)nximps/(double)ntokens
	  );
  tw->printf_raw("    - Disambig.   (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  (ntokens-nximps-nfumbles),100.0*(double)(ntokens-nximps-nfumbles)/(double)(ntokens-nximps),
	  nfumbles, 100.0*(double)(nfumbles)/(double)(ntokens-nximps)
	  );
}


/*--------------------------------------------------------------------------
 * token output
 *--------------------------------------------------------------------------*/
void put_tok_results(FILE *file, const mootToken &tok1, const mootToken &tok2)
{
  if (vlevel >= evlEverything
      || (vlevel >= evlMistakes
	  && (eval.isTokenMismatch()
	      || eval.isBestMismatch()
	      || (eval_file1
		  && (eval.isEmptyClass1()
		      || eval.isImpClass1()
		      || eval.isXImpClass1()))
	      || (eval_file2
		  && (eval.isEmptyClass2()
		      || eval.isImpClass2()
		      || eval.isXImpClass2())) ) ) )
    {
      fputs(eval.status_string().c_str(), file);
      fputc('\t', file);

      string t2s = twriter1->token2string(tok1);
      if (!t2s.empty()) t2s.erase(t2s.size()-1);
      fputs(t2s.c_str(), file);

      fputs("\t/\t", file);
      twriter2->put_token(tok2);
    }
}
