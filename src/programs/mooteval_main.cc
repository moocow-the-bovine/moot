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
#include "mooteval_cmdparser.h"
#include "cmdutil.h"

using namespace std;
using namespace moot;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "mooteval";

// files
cmdutil_file_info file1;
cmdutil_file_info file2;

// options & file-churning
gengetopt_args_info args;

//-- token i/o
TokenReader treader1(true,true);
TokenReader treader2(true,true);
TokenWriter twriter(false);

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

  //-- process arguments: files
  if (args.inputs_num < 2) {
    fprintf(stderr, "%s: you must specify two files to compare!\n", PROGNAME);
    cmdline_parser_print_help();
    exit(1);
  }

  //-- open: file1
  file1.name = args.inputs[0];
  if (!file1.open("r")) {
    fprintf(stderr, "%s: open failed for file '%s': %s",
	    PROGNAME, file1.name, strerror(errno));
    exit(2);
  }
  treader1.select_stream(file1.file, file1.name);

  //-- open: file2
  file2.name = args.inputs[1];
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
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GetMyOptions(argc,argv);

  //-- i/o & comparison
  mootSentence s1, s2;
  mootEval eval;
  int status;

  //-- information
  size_t ntokens = 0;     //-- total number of tokens
  size_t nsents = 0;      //-- total number of sentences
  size_t ntokmisses = 0;  //-- number of token-text mismatches
  size_t nbestmisses = 0; //-- number of best-tag mismatches
  size_t nempties1 = 0;   //-- number of empty analyses in file1
  size_t nimps1 = 0;      //-- number of impossible analyses in file1
  size_t nempties2 = 0;   //-- number of empty analyses in file2
  size_t nimps2 = 0;      //-- number of impossible analyses in file2

  //-- compare files
  do {
    s1 = treader1.get_sentence();
    s2 = treader2.get_sentence();

    //-- check for eof
    if (treader1.lexer.lasttyp == mootTokenLexer::TLEOF
	|| treader1.lexer.lasttyp == mootTokenLexer::TLEOF)
      {
	if ((treader1.lexer.lasttyp == mootTokenLexer::TLEOF
	     && treader2.lexer.lasttyp != mootTokenLexer::TLEOF)
	    ||
	    (treader1.lexer.lasttyp != mootTokenLexer::TLEOF
	     && treader2.lexer.lasttyp == mootTokenLexer::TLEOF))
	  {
	    fprintf(stderr, "%s: file lengths differ at lines %u/%u!\n",
		    PROGNAME, treader1.lexer.theLine, treader2.lexer.theLine);
	  }
	break;
      }

    //-- iterate over all tokens
    nsents++;
    mootSentence::const_iterator s1i, s2i;
    for (s1i=s1.begin()  ,  s2i=s2.begin();
	 s1i != s1.end() && s2i != s2.end();
	 s1i++           ,  s2i++)
      {
	ntokens++;
	status = eval.compareTokens(*s1i,*s2i);
	if (status & mootEval::MEF_TokMismatch) ntokmisses++;
	if (status & mootEval::MEF_BestMismatch) nbestmisses++;
	if (status & mootEval::MEF_EmptyClass1) nempties1++;
	if (status & mootEval::MEF_ImpClass1) nimps1++;
	if (status & mootEval::MEF_EmptyClass2) nempties2++;
	if (status & mootEval::MEF_ImpClass2) nimps2++;
      }

    //-- check sentence lengths
    if (s1i != s1.end() || s2i != s2.end()) {
      fprintf(stderr, "%s: sentence lengths differ at lines %u/%u!\n",
	      PROGNAME, treader1.lexer.theLine, treader2.lexer.theLine);
      continue;
    }
  } while (1);

  //-- summary
  fprintf(stdout,
	  "\n------------------------------------------------------------\n");
  fprintf(stdout, "%s Summary:\n", PROGNAME);
  fprintf(stdout, "  + File-1         : %s\n", file1.name);
  fprintf(stdout, "  + File-2         : %s\n", file2.name);
  fprintf(stdout, "  + General\n");
  fprintf(stdout, "    - NTokens      : %9u\n", ntokens);
  fprintf(stdout, "    - NSentences   : %9u\n", nsents);
  fprintf(stdout, "  + File-1\n");
  fprintf(stdout, "    - Classes\n");
  fprintf(stdout, "      * Empty      : %9u (%6.2f%%)\n",
	  nempties1, 100.0*(double)nempties1/(double)ntokens);
  fprintf(stdout, "      * Impossible : %9u (%6.2f%%)\n",
	  nimps1, 100.0*(double)nimps1/(double)ntokens);
  fprintf(stdout, "  + File-2\n");
  fprintf(stdout, "    - Classes\n");
  fprintf(stdout, "      * Empty      : %9u (%6.2f%%)\n",
	  nempties2, 100.0*(double)nempties2/(double)ntokens);
  fprintf(stdout, "      * Impossible : %9u (%6.2f%%)\n",
	  nimps2, 100.0*(double)nimps2/(double)ntokens);

  fprintf(stdout, "  + Token Text\n");
  fprintf(stdout, "    - Equal        : %9u (%6.2f%%)\n",
	  ntokens-ntokmisses,
	  100.0*(double)(ntokens-ntokmisses)/(double)ntokens);
  fprintf(stdout, "    - Different    : %9u (%6.2f%%)\n",
	  ntokmisses,
	  100.0*(double)(ntokmisses)/(double)ntokens);

  fprintf(stdout, "   + Best Tag\n");
  fprintf(stdout, "     - Equal       : %9u (%6.2f%%)\n",
	  ntokens-nbestmisses,
	  100.0*(double)(ntokens-nbestmisses)/(double)ntokens);
  fprintf(stdout, "     - Different   : %9u (%6.2f%%)\n",
	  nbestmisses,
	  100.0*(double)(nbestmisses)/(double)ntokens);

  fprintf(stdout,
	  "------------------------------------------------------------\n");

  //-- cleanup
  file1.close();
  file2.close();

  return 0;
}

