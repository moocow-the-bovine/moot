/*--------------------------------------------------------------------------
 * File: dwdst_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : main()
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "dwdst_cmdparser.h"
#include "dwdst.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "dwdst";

// options
gengetopt_args_info args;

// filenames
char *infile = "-";
char *outfile = "-";
char *symfile = "dwdst.sym";
char *fstfile = "dwdst.fst";

// files
FILE *in = NULL;
FILE *out = NULL;

// global structs
FSMSymSpec *syms = NULL;
FSM *morph = NULL;

#define DWDST_SYM_ATT_COMPAT true


/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  list<string> msglist;
  int fsmtype;

  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  // -- show banner
  if (args.verbose_given)
    fprintf(stderr,
	    "\n%s version %s by Bryan Jurish <moocow@ling.uni-potsdam.de>\n\n",
	    PROGNAME, VERSION);

  // -- output file
  if (strcmp(args.output_arg,"-") != 0) {
    in = fopen(args.output_arg, "w");
  } else {
    out = stdout;
  }
  if (!out) {
    fprintf(stderr,"%s: open failed for output-file '%s': %s\n", PROGNAME, args.output_arg, strerror(errno));
    exit(1);
  }

  // -- symbol spec
  if (args.verbose_given) {
    fprintf(stderr,"%s: loading symbols-file '%s'...", PROGNAME, args.symbols_arg);
  }
  syms = new FSMSymSpec(args.symbols_arg, &msglist, DWDST_SYM_ATT_COMPAT);
  if (!msglist.empty()) {
    fprintf(stderr,"\n%s Error: could not parse symbols-file '%s'\n", PROGNAME, args.symbols_arg);
    for (list<string>::iterator e = msglist.begin(); e != msglist.end(); e++) {
      fprintf(stderr,"%s\n",e->c_str());
    }
    exit(1);
  }
  if (args.verbose_given) {
    fprintf(stderr," symbols-file loaded.\n");
  }

  // -- morphology transducer
  if (args.verbose_given) {
    fprintf(stderr,"%s: loading morphology FST '%s'...", PROGNAME, args.morph_arg);
  }
  morph = new FSM(args.morph_arg);
  fsmtype = morph->fsm_type();
  if (!morph || !morph->representation()
      // HACK!
      || (fsmtype != FSMTypeTransducer &&
	  fsmtype != FSMTypeWeightedTransducer &&
	  fsmtype != FSMTypeSubsequentialTransducer &&
	  fsmtype != FSMTypeAcceptor &&
	  fsmtype != FSMTypeWeightedAcceptor))
    {   
      fprintf(stderr,"\n%s ERROR: load failed for morphology FST '%s'.\n", PROGNAME, args.morph_arg);
      exit(1);
    }
  if (args.verbose_given) {
    fprintf(stderr," morphology FST loaded.\n");
  }
  
  // link morph-FST to symbols file
  if (!morph->fsm_use_symbol_spec(syms)) {
    fprintf(stderr,"%s ERROR: could not use symbols from '%s' in FST from '%s'\n",
	    PROGNAME, args.symbols_arg, args.morph_arg);
    exit(1);
  }
}

/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  unsigned int i;

  GetMyOptions(argc,argv);

  if (args.words_given) {
    fprintf(out, "# %s: File: command-line\n\n", PROGNAME);
    for (i = 0; i < args.inputs_num; i++) {
      dwdst_tag_token(args.inputs[i], out, morph, syms, args.avm_given);
    }
  }
  else if (args.inputs_num == 0) {
    fprintf(out, "# %s: File: stdin\n\n", PROGNAME);
    dwdst_tag_stream(stdin, out, morph, syms, args.avm_given);
  }
  else {
    for (i = 0; i < args.inputs_num; i++) {
      fprintf(out, "# %s: File: %s\n\n", PROGNAME, args.inputs[i]);
      if (!(in = fopen(args.inputs[i], "r"))) {
	fprintf(stderr, "%s: open failed for input-file '%s': %s", PROGNAME, args.inputs[i], strerror(errno));
	exit(1);
      }
      if (args.verbose_given) {
	fprintf(stderr,"%s: parsing file '%s'... ", PROGNAME, args.inputs[i]);
      }
      dwdst_tag_stream(in, out, morph, syms, args.avm_given);
      fclose(in);
      if (args.verbose_given) {
	fprintf(stderr," done.\n");
      }
    }
  }

  return 0;
}

