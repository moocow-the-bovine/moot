/*--------------------------------------------------------------------------
 * File: dwdst_fstgen_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger-trainer for DWDS project : main()
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include <dwdstTrainer.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "dwdst_fstgen_cmdparser.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "dwdst-fstgen";

// options & file-churning
gengetopt_args_info args;

// -- filenames
char *symfile = "dwdst.sym";

// -- global classes/structs
dwdstTrainer dwdstt;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  // -- sanity check
  if (!args.unknown_given && !args.disambig_given) {
    fprintf(stderr, "%s Error: you must specify one of the options '--unknown' or '--disambig'!\n",
	    PROGNAME);
    exit(1);
  }
  if (args.unknown_given && args.disambig_given) {
    fprintf(stderr, "%s Error: you can only specify one of the options '--unknown' or '--disambig'!\n",
	    PROGNAME);
    exit(1);
  }

  // -- show banner
  if (args.verbose_arg > 0)
    fprintf(stderr,
	    "\n%s version %s by Bryan Jurish <moocow@ling.uni-potsdam.de>\n\n",
	    PROGNAME, VERSION);

  // -- generator object setup : flags
  dwdstt.want_avm = false;
  dwdstt.want_features = !args.tags_only_given;
  dwdstt.verbose  = (args.verbose_arg > 0);

  // -- generator object setup : symbols
  if (args.verbose_arg > 0) fprintf(stderr, "%s: loading symbols-file '%s'...", PROGNAME, args.symbols_arg);
  if (!dwdstt.load_symbols(args.symbols_arg)) {
    fprintf(stderr,"\n%s: load FAILED for symbols-file '%s'\n", PROGNAME, args.symbols_arg);
  } else if (args.verbose_arg > 0) {
    fprintf(stderr," loaded.\n");
  }

  // -- trainer object setup : all-tags-list
  if (args.verbose_arg > 0)
    fprintf(stderr, "%s: generating universal PoS-tag list from file '%s'...",
	    PROGNAME, args.pos_tag_list_given ? args.pos_tag_list_arg : "(none)");
  if (dwdstt.get_all_pos_tags(args.pos_tag_list_given ? args.pos_tag_list_arg : NULL).empty()) {
    fprintf(stderr,"\n%s: generation FAILED for universal PoS-tag list from file '%s'\n",
	    PROGNAME, args.pos_tag_list_given ? args.pos_tag_list_arg : "(none)");
  } else if (args.verbose_arg > 0) {
    fprintf(stderr," generated.\n");
  }
  // -- trainer object setup : open-class-list
  if (args.verbose_arg > 0)
    fprintf(stderr, "%s: generating open-class PoS-tag list from file '%s'...",
	    PROGNAME, args.open_class_list_given ? args.open_class_list_arg : "(none)");
  if (dwdstt.get_open_class_tags(args.open_class_list_given ? args.open_class_list_arg : NULL).empty()) {
    fprintf(stderr,"\n%s: generation FAILED for open-class PoS-tag list from file '%s'\n",
	    PROGNAME, args.open_class_list_given ? args.open_class_list_arg : "(none)");
  } else if (args.verbose_arg > 0) {
    fprintf(stderr," generated.\n");
  }

  // -- filename defaults
  if (!args.output_file_given) {
    if (args.unknown_given) {
      args.output_file_arg = strdup("dwdst-unknown.fst");
    }
    else if (args.disambig_given) {
      args.output_file_arg = strdup("dwdst-disambig.fst");
    }
  }

  // -- report
  if (args.verbose_arg > 0) {
    fprintf(stderr, "%s: Initialization complete\n\n", PROGNAME);
  }
}
  


/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GetMyOptions(argc,argv);

  // -- the guts

  // -- FSA generation
  if (args.unknown_given) {
    //-------------------------------------------
    // -- unknown-analysis FSA
    //-------------------------------------------
    if (args.verbose_arg > 0)
      fprintf(stderr, "%s: generating analysis FSA for unknown tokens...", PROGNAME);
    if (!dwdstt.generate_unknown_fsa()) {
      fprintf(stderr,"\n%s: generation FAILED for unknown-token analysis FSA!\n", PROGNAME);
    } else if (args.verbose_arg > 0) {
      fprintf(stderr," generated.\n");
    }

    // -- save unknown FSA
    if (args.verbose_arg > 0) {
      fprintf(stderr, "%s: saving analysis FSA for unknown tokens to file '%s'...",
	      PROGNAME, args.output_file_arg);
    }
    //FSM *ufsa_t =
    dwdstt.ufsa->fsm_convert(FSM::FSMCTTable);
    FSM *ufsa_t = dwdstt.ufsa;
    if (!ufsa_t->fsm_save_to_binary_file(args.output_file_arg)) {
      fprintf(stderr, "\n%s Error: could not save analysis FSA for unknown tokens to file '%s'.\n",
	      PROGNAME, args.output_file_arg);
      exit(1);
    } else if (args.verbose_arg > 0) {
      fprintf(stderr, " saved.\n");
    }
    //ufsa_t->fsm_free();
  }
  else if (args.disambig_given) {
    //-------------------------------------------
    // -- disambiguation FSA
    //-------------------------------------------
    fprintf(stderr,
	    "%s Error: generation of disambiguation-FSAs is not yet implemented!\n",
	    PROGNAME);
    exit(1);
  }

  return 0;
}

