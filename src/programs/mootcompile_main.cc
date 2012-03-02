/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2002-2012 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootcompile_main.cc
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + HMM PoS tagger/disambiguator for DWDS project : model compiler
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <string>

#include "computils.h"
#include "mootcompile_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals (see also computils.h)
 *--------------------------------------------------------------------------*/

// options & file-churning
gengetopt_args_info  args;

// -- files
mofstream out;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- load environmental defaults
  cmdline_parser_envdefaults(&args);

  //-- locale
  moot_setlocale();

  //-- verbose
  vlevel = args.verbose_arg;

  //-- sanity check
  if (args.inputs_num <= 0)
    moot_croak("%s: cowardly refusing to compile empty model!\n", PROGNAME);
  for (int i=1; i < args.inputs_num; ++i) {
    moot_msg(vlevel,vlWarnings,"%s: Warning: ignoring non-option argument #%i `%s'\n", PROGNAME, (i+1), args.inputs[i]);
  }

  //-- show banner
  if (!args.no_banner_given)
    moot_msg(vlevel, vlInfo, moot_program_banner(PROGNAME, PACKAGE_VERSION, "Bryan Jurish <moocow@cpan.org>").c_str());

  //-- output file
  if (!out.open(args.output_arg,"wb")) {
    moot_croak("%s: open failed for output-file '%s': %s\n", PROGNAME, out.name.c_str(), strerror(errno));
  }
  out.close(); //-- close again: HMM will write it itself

  //-- setup computils.h HmmSpec (model constants etc)
  hs.hash_ngrams = args.hash_ngrams_arg;
  hs.relax = args.relax_arg;
  hs.use_classes = args.use_classes_arg;
  hs.use_flavors = args.use_flavors_arg;
  hs.unknown_token_name = args.unknown_token_arg;
  hs.unknown_tag_name = args.unknown_tag_arg;
  hs.eos_tag = args.eos_tag_arg;
  hs.unknown_lex_threshhold = args.unknown_threshhold_arg;
  hs.unknown_class_threshhold = args.class_threshhold_arg;
  hs.beam_width = args.beam_width_arg;
  hs.nlambdas = args.nlambdas_arg;
  hs.wlambdas = args.wlambdas_arg;
  hs.clambdas = args.clambdas_arg;
  hs.trie_depth = args.trie_depth_arg;
  hs.trie_threshhold = args.trie_threshhold_arg;
  hs.trie_theta = args.trie_theta_arg;
  hs.trie_args_given = (args.trie_depth_given || args.trie_threshhold_given || args.trie_theta_given);
  //
  //-- runtime options (not really needed here)
  hs.save_ambiguities = 0;
  hs.save_mark_unknown = 0;
  hs.ndots = 0;
}


/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
 try {
  PROGNAME = "mootcompile";
  GetMyOptions(argc,argv);

  //-- the guts : load & compile input model (single model ONLY!)
  if (!load_hmm(args.inputs[0])) 
    moot_croak("%s: load FAILED for model `%s' -- aborting!\n", PROGNAME, args.inputs[0]);

  //-- dump binary model
  moot_msg(vlevel,vlProgress,"%s: saving binary HMM `%s' ...", PROGNAME, out.name.c_str());
  if (!hmm.save(out.name.c_str(), args.compress_arg))
    moot_croak("\n%s: binary HMM dump FAILED \n", PROGNAME);
  moot_msg(vlevel,vlProgress," saved.\n");

  //-- summary
  char cmts[3] = "%%";
  if (vlevel >= vlInfo) {
    fprintf(stderr, "\n%s %s Summary:\n", cmts, PROGNAME);
    fprintf(stderr, "%s   Unknown Token     : %s\n", cmts, hmm.tokids.id2name(0).c_str());
    fprintf(stderr, "%s   Unknown Tag       : %s\n", cmts, hmm.tagids.id2name(0).c_str());
    fprintf(stderr, "%s   Border Tag        : %s\n", cmts, hmm.tagids.id2name(hmm.start_tagid).c_str());
    fprintf(stderr, "%s   N-Gram lambdas    : lambda1=%g, lambda2=%g", cmts, hmm.nglambda1, hmm.nglambda2);
#ifdef moot_USE_TRIGRAMS
    fprintf(stderr, " lambda3=%g", hmm.nglambda3);
#endif
    fprintf(stderr, "\n");
    fprintf(stderr, "%s   Hash n-grams?     : %s\n", cmts, (hmm.hash_ngrams ? "yes" : "no"));
    fprintf(stderr, "%s   Lex. Threshhold   : %g\n", cmts, hmm.unknown_lex_threshhold);
    fprintf(stderr, "%s   Lexical lambdas   : lambdaw0=%g, lambdaw1=%g\n", cmts, hmm.wlambda0, hmm.wlambda0);
    fprintf(stderr, "%s   Use classes?      : %s\n", cmts, hmm.use_lex_classes ? "yes" : "no");
    fprintf(stderr, "%s   Use flavors?      : %s\n", cmts, hmm.use_flavors ? "yes" : "no");
    fprintf(stderr, "%s   Num/Flavors       : %u\n", cmts, hmm.taster.size());
    fprintf(stderr, "%s   Class Threshhold  : %g\n", cmts, hmm.unknown_class_threshhold);
    fprintf(stderr, "%s   Class lambdas     : lambdac0=%g, lambdac1=%g\n", cmts, hmm.clambda0, hmm.clambda1);
    fprintf(stderr, "%s   Beam Width        : %g\n", cmts, hmm.beamwd);
  }
 }
 catch (exception &e) {
   fprintf(stderr, "%s: Exception: %s\n", PROGNAME, e.what());
   abort();
 }

  return 0;
}
