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

#include <mootTokenIO.h>
#include <mootEval.h>

#include <mootUtils.h>
#include "mooteval_cmdparser.h"

using namespace std;
using namespace moot;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "mooteval";

// files
cmdutil_file_info file1;
cmdutil_file_info file2;
cmdutil_file_info out;

// options & file-churning
gengetopt_args_info args;

//-- token i/o
TokenReaderCookedFile treader1(true);
TokenReaderCookedFile treader2(true);
TokenWriterCookedString twriter1(false);
TokenWriterCookedFile twriter2(false);
//mootSentence s1, s2;
mootTokFlavor tf1, tf2, tf1prev, tf2prev;

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
	    "\n%s version %s by Bryan Jurish <jurish@ling.uni-potsdam.de>\n\n",
	    PROGNAME, VERSION);

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
  out.name = args.output_arg;
  if (strcmp(out.name,"-") == 0) out.name = "<stdout>";
  if (!out.open("w")) {
    fprintf(stderr,"%s: open failed for output-file '%s': %s\n",
	    PROGNAME, out.name, strerror(errno));
    exit(1);
  }
  twriter2.output_file(out.file);

  //-- process arguments: input files
  if (args.inputs_num < 2) {
    fprintf(stderr, "%s: you must specify two files to compare!\n", PROGNAME);
    cmdline_parser_print_help();
    exit(1);
  }

  //-- open: file1
  if (strcmp(args.inputs[0], "-")==0) file1.name = "<stdin>";
  else file1.name = args.inputs[0];
  if (!file1.open("r")) {
    fprintf(stderr, "%s: open failed for file '%s': %s",
	    PROGNAME, file1.name, strerror(errno));
    exit(2);
  }
  treader1.select_stream(file1.file, file1.name);

  //-- open: file2
  if (strcmp(args.inputs[1], "-")==0) file2.name = "<stdin>";
  else file2.name = args.inputs[1];
  if (!file2.open()) {
    fprintf(stderr, "%s: open failed for file '%s': %s",
	    PROGNAME, file2.name, strerror(errno));
    exit(2);
  }
  treader2.select_stream(file2.file, file2.name);

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
void print_summary(FILE *file);

void print_summary_file(FILE *outfile,
			size_t ntokens, size_t nanls,
			size_t nempties, size_t nimps, size_t nximps,
			size_t nsaves, size_t nfumbles);


/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GetMyOptions(argc,argv);

  //-- Dummy token
  mootToken tokDummy("%% (EMPTY)");
  tokDummy.flavor(TF_TOKEN); //-- really a comment

  mootToken tokEOS("%% (EOS)");
  tokDummy.flavor(TF_TOKEN); //-- really a comment

  mootToken tokEOF("%% (EOF)");
  tokDummy.flavor(TF_TOKEN); //-- really a comment

  //-- input configuration : ignore comments completely
  //treader1.lexer.ignore_comments = true;
  //treader2.lexer.ignore_comments = true;

  //-- compare files
  tf1prev = TF_EOS;
  tf2prev = TF_EOS;
  for (tf1=treader1.get_token()  , tf2=treader2.get_token();
       tf1 != TF_EOF            || tf2 != TF_EOF;
       /*treader1.get_token(), treader2.get_token()*/ )
    {
      //-- assign token aliases
      const mootToken &tok1 = (tf1 == TF_EOS
			       ? tokEOS
			       : (tf1 == TF_EOF
				  ? tokEOF
				  : treader1.token()));
      const mootToken &tok2 = (tf2 == TF_EOS
			       ? tokEOS
			       : (tf2 == TF_EOF
				  ? tokEOF
				  : treader2.token()));

      //-- check for comments
      if (tf1 == TF_COMMENT || tf2 == TF_COMMENT) {
	eval.clear();
	if (tf1==tf2) {
	  //-- comments in both files
	  put_tok_results(out.file, tok1, tok2);
	  //tf1prev = tf1;
	  //tf2prev = tf2;
	  tf1 = treader1.get_token();
	  tf2 = treader2.get_token();
	}
	else if (tf1==TF_COMMENT) {
	  //-- comment in file1 only
	  put_tok_results(out.file, tok1, tok2);
	  //tf1prev = tf1;
	  tf1 = treader1.get_token();
	}
	else if (tf2==TF_COMMENT) {
	  put_tok_results(out.file, tok1, tok2);
	  //tf2prev = tf2;
	  tf2 = treader2.get_token();
	}
	continue;
      }

      //-- check for EOS
      else if (tf1 == TF_EOS || tf2 == TF_EOS) {
	if (tf1==tf2) {
	  //-- eos: match
	  nsents++;
	  if (vlevel >= evlEverything) fputc('\n', out.file);
	  tf1prev = tf1;
	  tf1 = treader1.get_token();
	  tf2prev = tf2;
	  tf2 = treader2.get_token();
	  continue;
	}
	else if (tf1prev==TF_EOS || tf2prev==TF_EOS) {
	  //-- ignore consecutive EOS tokens
	  if (tf1 == TF_EOS) {
	    tf1prev = tf1;
	    tf1 = treader1.get_token();
	  }
	  if (tf2 == TF_EOS) {
	    tf2prev = tf2;
	    tf2 = treader2.get_token();
	  }
	  continue;
	}
	//-- report real eos mismatch
	fprintf(stderr, "%s: sentence lengths differ at lines %u/%u!\n",
		PROGNAME, treader1.lexer.theLine, treader2.lexer.theLine);
	if (tf1 == TF_EOS) {
	  tf1prev = tf1;
	  tf1 = treader1.get_token();
	}
	if (tf2 == TF_EOS) {
	  tf2prev = tf2;
	  tf2 = treader2.get_token();
	}
	continue;
      }

      //-- check for EOF
      else if (tf1 == TF_EOF || tf2 == TF_EOF) {
	if (tf1==tf2) break; //-- eof: match

	else if (tf1 != TF_EOF) {
	  put_tok_results(out.file, tok1, tok2);
	  tf1prev = tf1;
	  tf1 = treader1.get_token();
	  if (tf1prev == TF_EOS) continue; //-- skip eof/eos mismatch
	}
	else if (tf2 != TF_EOF) {
	  put_tok_results(out.file, tok1, tok2);
	  tf2prev = tf2;
	  tf2 = treader2.get_token();
	  if (tf2prev == TF_EOS) continue; //-- skip eof/eos mismatch
	}

	//-- report real eof mismatch
	fprintf(stderr, "%s: file lengths differ at lines %u/%u!\n",
		PROGNAME, treader1.lexer.theLine, treader2.lexer.theLine);
	continue;
      }

      //--------------------------------------------------------
      //-- "plain" tokens: compare
      ntokens++;
      nans1 += tok1.analyses().size();
      nans2 += tok2.analyses().size();

      eval.compareTokens(tok1,tok2);
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
      put_tok_results(out.file, tok1, tok2);
      
      //-- get next token in each file
      tf1prev = tf1;
      tf2prev = tf2;
      tf1 = treader1.get_token();
      tf2 = treader2.get_token();
    }

  //-- summary
  print_summary(out.file);

  if (vlevel >= evlBasic && out.file != stdout) {
    print_summary(stderr);
  }

  //-- cleanup
  file1.close();
  file2.close();
  out.close();

  return 0;
}



/*--------------------------------------------------------------------------
 * summary (new)
 *--------------------------------------------------------------------------*/
void print_summary(FILE *outfile)
{
  if (ntokens==0) ntokens = 1;
  fprintf(outfile,
	  "%%%%-------------------------------------------------------------------\n");
  fprintf(outfile, "%%%% %s Summary:\n", PROGNAME);

  //----------------------------------------------------
  // General
  fprintf(outfile, "%%%%  + NTokens             : %9u\n", ntokens);
  fprintf(outfile, "%%%%  + NSentences          : %9u\n", nsents);

  //----------------------------------------------------
  // File-1
  fprintf(outfile, "%%%%  + File-1              : %s\n", file1.name);
  if (vlevel >= evlExtended && eval_file1)
    print_summary_file(outfile, ntokens, nans1, nempties1,
		       nimps1, nximps1, nsaves1, nfumbles1);

  //----------------------------------------------------
  // File-2
  fprintf(outfile, "%%%%  + File-2              : %s\n", file2.name);
  if (vlevel >= evlExtended && eval_file2)
    print_summary_file(outfile, ntokens, nans2, nempties2,
		       nimps2, nximps2, nsaves2, nfumbles2);

  //----------------------------------------------------
  // Accuracy
  fprintf(outfile, "%%%%  + Tokens Equal  (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  ntokens-ntokmisses, 100.0*(double)(ntokens-ntokmisses)/(double)ntokens,
	  ntokmisses, 100.0*(double)(ntokmisses)/(double)ntokens);

  fprintf(outfile, "%%%%  + Tags Equal    (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  ntokens-nbestmisses, 100.0*(double)(ntokens-nbestmisses)/(double)ntokens,
	  nbestmisses, 100.0*(double)(nbestmisses)/(double)ntokens);

  fprintf(outfile,
	  "%%%%-------------------------------------------------------------------\n");
}




/*--------------------------------------------------------------------------
 * summary (file)
 *--------------------------------------------------------------------------*/
void print_summary_file(FILE *outfile,
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
  fprintf(outfile, "%%%%    - Avg. Class Size   : %39.2f\n",
	  (double)nanls/(double)(ntokens-nempties)
	  );
  fprintf(outfile, "%%%%    - Class Given (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  ntokens-nempties, 100.0*(double)(ntokens-nempties)/(double)ntokens,
	  nempties, 100.0*(double)nempties/(double)ntokens
	  );
  fprintf(outfile, "%%%%    - Saves       (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  nsaves, 100.0*(double)nsaves/(double)nempties,
	  nempties-nsaves, 100.0*(double)(nempties-nsaves)/(double)nempties
	  );
  fprintf(outfile, "%%%%    - Int. Cover  (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  ntokens-nimps, 100.0*(double)(ntokens-nimps)/(double)ntokens,
	  nimps, 100.0*(double)nimps/(double)ntokens
	  );
  fprintf(outfile, "%%%%    - Ext. Cover  (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  ntokens-nximps, 100.0*(double)(ntokens-nximps)/(double)ntokens,
	  nximps, 100.0*(double)nximps/(double)ntokens
	  );
  fprintf(outfile, "%%%%    - Disambig.   (+/-) : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
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
      fputs(twriter1.token2string(tok1).c_str(), file);
      fputs("\t/\t", file);
      twriter2.put_token(tok2);
    }
}
