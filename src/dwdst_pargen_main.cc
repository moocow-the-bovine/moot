/*--------------------------------------------------------------------------
 * File: dwdst_pargen_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger-trainer for DWDS project : main()
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "dwdst_pargen_cmdparser.h"
#include "cmdutil.h"
#include "dwdst_pargen.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "dwdst_pargen";

// options & file-churning
gengetopt_args_info args;
cmdutil_file_churner churner;

// -- filenames
char *symfile = "dwdst.sym";
char *fstfile = "dwdst.fst";

// -- files
cmdutil_file_info out;

// -- global classes/structs
dwds_param_generator pargen;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  // -- show banner
  if (args.verbose_arg > 0)
    fprintf(stderr,
	    "\n%s version %s by Bryan Jurish <moocow@ling.uni-potsdam.de>\n\n",
	    PROGNAME, VERSION);

  // -- output file
  out.name = args.output_arg;
  if (strcmp(out.name,"-") == 0) out.name = "<stdout>";
  if (!out.open("w")) {
    fprintf(stderr,"%s: open failed for output-file '%s': %s\n",
	    PROGNAME, out.name, strerror(errno));
    exit(1);
  }

  // -- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = args.list_given;

  // -- trainer object setup : flags
  //dwdst.want_avm = args.avm_given;
  pargen.want_avm = false;
  pargen.want_features = !args.tags_only_given;
  pargen.verbose  = (args.verbose_arg > 0);
  pargen.wdsep = args.word_separator_arg;

  // -- trainer object setup : symbols
  if (args.verbose_arg > 0) fprintf(stderr, "%s: loading symbols-file '%s'...", PROGNAME, args.symbols_arg);
  if (!pargen.load_symbols(args.symbols_arg)) {
    fprintf(stderr,"\n%s: load FAILED for symbols-file '%s'\n", PROGNAME, args.symbols_arg);
  } else if (args.verbose_arg > 0) {
    fprintf(stderr," loaded.\n");
  }

  // -- trainer object setup : unknown-analysis FSA
  if (args.verbose_arg > 0)
      fprintf(stderr, "%s: loading unknown-token analysis FSA file '%s'...",
	      PROGNAME, args.unknown_fsa_arg);
  if (!pargen.load_unknown_fsa(args.unknown_fsa_arg)) {
      fprintf(stderr,"\n%s: load FAILED for unknown-token analysis FSA file '%s'.\n",
	      PROGNAME, args.unknown_fsa_arg);
  } else if (args.verbose_arg > 0) {
      fprintf(stderr," loaded.\n");
  }


  // -- trainer object setup : morphology FST
  if (args.verbose_arg > 0) fprintf(stderr, "%s: loading morphology FST '%s'...", PROGNAME, args.morph_arg);
  if (!pargen.load_morph(args.morph_arg)) {
    fprintf(stderr,"\n%s: load FAILED for morphology FST '%s'\n", PROGNAME, args.morph_arg);
  } else if (args.verbose_arg > 0) {
    fprintf(stderr," loaded.\n");
  }

  // -- trainer object setup : kmax , eos
  pargen.kmax = args.kgram_max_arg;
  pargen.eos = args.eos_string_arg;

  // -- trainer object setup : link morph-FST to symbols file
  if (!pargen.morph->fsm_use_symbol_spec(pargen.syms) ||
      !pargen.ufsa->fsm_use_symbol_spec(pargen.syms))
      {
	  fprintf(stderr,"%s ERROR: could not use symbols from '%s' in FST from '%s'\n",
		  PROGNAME, args.symbols_arg, args.morph_arg);
	  exit(1);
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
  int nfiles = 0;

  GetMyOptions(argc,argv);

   // -- the guts
  if (args.words_given) {
    if (args.verbose_arg > 0) {
      fprintf(stderr, "%s: Training from command-line tokens\n\n", PROGNAME);
    }
    pargen.train_from_strings(args.inputs_num, args.inputs, out.file);
  } else {
    if (args.verbose_arg > 0) {
      fprintf(stderr, "%s: Training from input files... %s",
	      PROGNAME, args.verbose_arg > 1 ? "\n" : "");
    }
    // -- big loop
    for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
      if (args.verbose_arg > 0) {
	nfiles++;
	if (args.verbose_arg > 1) {
	  fprintf(stderr,"%s: analyzing file '%s'... ", PROGNAME, churner.in.name);
	  fflush(stderr);
	}
      }
#    ifdef DWDST_PARGEN_DEBUG
      fprintf(out.file, "\n# %s (DEBUG): File: %s\n\n", PROGNAME, churner.in.name);
#    endif
      
      pargen.train_from_stream(churner.in.file, out.file);

      if (args.verbose_arg > 1) {
	fprintf(stderr," done.\n");
	fflush(stderr);
      }
    }
    if (args.verbose_arg == 1) {
      fprintf(stderr, " done.\n");
      fflush(stderr);
    }
  }

#ifdef DWDST_PARGEN_DEBUG
  fprintf(out.file, "\n");
#endif

  // -- paramater file
  if (args.verbose_arg > 0 && out.file != stdout) {
    fprintf(stderr, "%s: Generating parameter file '%s'... ",
	    PROGNAME, out.name);
  }
  pargen.write_param_file(out.file);
  if (args.verbose_arg > 0 && out.file != stdout) {
    out.close();
    fprintf(stderr, "done.\n");
  } else {
    out.close();
  }

  // -- summary
  if (args.verbose_arg > 0) {
      // -- print summary
      fprintf(stderr, "\n-----------------------------------------------------\n");
      fprintf(stderr, "%s Summary:\n", PROGNAME);
      fprintf(stderr, "  + Files processed  : %d\n", nfiles);
      fprintf(stderr, "  + Tokens processed : %d\n", pargen.ntokens);
      fprintf(stderr, "-----------------------------------------------------\n");
  }

  return 0;
}

