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

//----------------------------------------------------------------------
// spec overrides
namespace moot {
  template<> const char* HmmSpec<gengetopt_args_info,mootHMM>::model_arg()   { return args.inputs[0]; }
  template<> bool	 HmmSpec<gengetopt_args_info,mootHMM>::model_given() { return args.inputs_num > 0; }

  template<> int  HmmSpec<gengetopt_args_info,mootHMM>::save_ambiguities_arg() { return false; };
  template<> bool HmmSpec<gengetopt_args_info,mootHMM>::save_ambiguities_given() { return false; };

  template<> int  HmmSpec<gengetopt_args_info,mootHMM>::save_mark_unknown_arg() { return false; };
  template<> bool HmmSpec<gengetopt_args_info,mootHMM>::save_mark_unknown_given() { return false; };

  template<> size_t HmmSpec<gengetopt_args_info,mootHMM>::ndots_arg() { return 0; };
  template<> bool   HmmSpec<gengetopt_args_info,mootHMM>::ndots_given() { return false; };
};

/*--------------------------------------------------------------------------
 * Globals (see also computils.h)
 *--------------------------------------------------------------------------*/

// options & file-churning
gengetopt_args_info  args;
mootHMM hmm;
HmmSpec<gengetopt_args_info,mootHMM> spec;

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
  spec.args = args;
  spec.hmmp = &hmm;
  if (!spec.load_hmm(false))
    moot_croak("%s: compile FAILED for text model `%s' -- aborting\n", PROGNAME, spec.model_arg());

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
    fprintf(stderr, "%s   Relax?            : %s\n", cmts, (hmm.relax ? "yes" : "no"));
    fprintf(stderr, "%s   Lex. Threshhold   : %g\n", cmts, hmm.unknown_lex_threshhold);
    fprintf(stderr, "%s   Lexical lambdas   : lambdaw0=%g, lambdaw1=%g\n", cmts, hmm.wlambda0, hmm.wlambda0);
    fprintf(stderr, "%s   Use classes?      : %s\n", cmts, hmm.use_lex_classes ? "yes" : "no");
    fprintf(stderr, "%s   Use flavors?      : %s\n", cmts, hmm.use_flavors ? "yes" : "no");
    fprintf(stderr, "%s   Num/Flavors       : %zu\n", cmts, hmm.taster.size());
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
