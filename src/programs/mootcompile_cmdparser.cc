/* -*- Mode: C -*-
 *
 * File: mootcompile_cmdparser.c
 * Description: Code for command-line parser struct gengetopt_args_info.
 *
 * File autogenerated by optgen.perl version 0.06
 * generated with the following command:
 * /usr/local/bin/optgen.perl -u -l --nopod -F mootcompile_cmdparser mootcompile.gog
 *
 * The developers of optgen.perl consider the fixed text that goes in all
 * optgen.perl output files to be in the public domain:
 * we make no copyright claims on it.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

/* If we use autoconf/autoheader.  */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif

/* Allow user-overrides for PACKAGE and VERSION */
#ifndef PACKAGE
#  define PACKAGE "PACKAGE"
#endif

#ifndef VERSION
#  define VERSION "VERSION"
#endif


#ifndef PROGRAM
# define PROGRAM "mootcompile"
#endif

/* #define cmdline_parser_DEBUG */

/* Check for "configure's" getopt check result.  */
#ifndef HAVE_GETOPT_LONG
# include "getopt.h"
#else
# include <getopt.h>
#endif

#include "mootcompile_cmdparser.h"


/* user code section */

/* end user  code section */


void
cmdline_parser_print_version (void)
{
  printf("mootcompile (%s %s) by Bryan Jurish <moocow@cpan.org>\n", PACKAGE, VERSION);
}

void
cmdline_parser_print_help (void)
{
  cmdline_parser_print_version ();
  printf("\n");
  printf("Purpose:\n");
  printf("  moocow's HMM part-of-speech tagger/disambiguator: model compiler.\n");
  printf("\n");
  
  printf("Usage: %s [OPTIONS]... MODEL(s)\n", "mootcompile");
  
  printf("\n");
  printf(" Arguments:\n");
  printf("   MODEL(s)  Text-format input models.\n");
  
  printf("\n");
  printf(" Options:\n");
  printf("   -h        --help                       Print help and exit.\n");
  printf("   -V        --version                    Print version and exit.\n");
  printf("   -cFILE    --rcfile=FILE                Read an alternate configuration file.\n");
  printf("   -vLEVEL   --verbose=LEVEL              Verbosity level.\n");
  printf("   -B        --no-banner                  Suppress initial banner message (implied at verbosity levels <= 2)\n");
  printf("   -oFILE    --output=FILE                Specify output file (default=stdout).\n");
  printf("   -zLEVEL   --compress=LEVEL             Compression level for output file.\n");
  printf("\n");
  printf(" HMM Options:\n");
  printf("   -gBOOL    --hash-ngrams=BOOL           Whether to hash stored n-grams (default=no)\n");
  printf("   -aLEN     --trie-depth=LEN             Maximum depth of suffix trie.\n");
  printf("   -AFREQ    --trie-threshhold=FREQ       Frequency upper bound for trie inclusion.\n");
  printf("             --trie-theta=FLOAT           Suffix backoff coefficient.\n");
  printf("   -LBOOL    --use-classes=BOOL           Whether to use lexical class-probabilities.\n");
  printf("   -FBOOL    --use-flavors=BOOL           Whether to use token 'flavor' heuristics (default=1 (true))\n");
  printf("   -RBOOL    --relax=BOOL                 Whether to relax token-tag associability (default=1 (true))\n");
  printf("   -NFLOATS  --nlambdas=FLOATS            N-Gram smoothing constants (default=estimate)\n");
  printf("   -WFLOATS  --wlambdas=FLOATS            Lexical smoothing constants (default=estimate)\n");
  printf("   -CFLOATS  --clambdas=FLOATS            Lexical-class smoothing constants (default=estimate)\n");
  printf("   -tDOUBLE  --unknown-threshhold=DOUBLE  Freq. threshhold for 'unknown' lexical probabilities\n");
  printf("   -TDOUBLE  --class-threshhold=DOUBLE    Freq. threshhold for 'unknown' class probabilities\n");
  printf("   -uNAME    --unknown-token=NAME         Symbolic name of the 'unknown' token\n");
  printf("   -UNAME    --unknown-tag=NAME           Symbolic name of the 'unknown' tag\n");
  printf("   -eTAG     --eos-tag=TAG                Specify boundary tag (default=__$)\n");
  printf("   -ZDOUBLE  --beam-width=DOUBLE          Specify cutoff factor for beam pruning\n");
}

#if defined(HAVE_STRDUP) || defined(strdup)
# define gog_strdup strdup
#else
/* gog_strdup(): automatically generated from strdup.c. */
/* strdup.c replacement of strdup, which is not standard */
static char *
gog_strdup (const char *s)
{
  char *result = (char*)malloc(strlen(s) + 1);
  if (result == (char*)0)
    return (char*)0;
  strcpy(result, s);
  return result;
}
#endif /* HAVE_STRDUP */

/* clear_args(args_info): clears all args & resets to defaults */
static void
clear_args(struct gengetopt_args_info *args_info)
{
  args_info->rcfile_arg = NULL; 
  args_info->verbose_arg = 4; 
  args_info->no_banner_flag = 0; 
  args_info->output_arg = gog_strdup("-"); 
  args_info->compress_arg = -1; 
  args_info->hash_ngrams_arg = 0; 
  args_info->trie_depth_arg = 0; 
  args_info->trie_threshhold_arg = 10; 
  args_info->trie_theta_arg = 0; 
  args_info->use_classes_arg = 1; 
  args_info->use_flavors_arg = 1; 
  args_info->relax_arg = 1; 
  args_info->nlambdas_arg = NULL; 
  args_info->wlambdas_arg = NULL; 
  args_info->clambdas_arg = NULL; 
  args_info->unknown_threshhold_arg = 1.0; 
  args_info->class_threshhold_arg = 1.0; 
  args_info->unknown_token_arg = gog_strdup("@UNKNOWN"); 
  args_info->unknown_tag_arg = gog_strdup("UNKNOWN"); 
  args_info->eos_tag_arg = gog_strdup("__$"); 
  args_info->beam_width_arg = 1000; 
}


int
cmdline_parser (int argc, char * const *argv, struct gengetopt_args_info *args_info)
{
  int c;	/* Character of the parsed option.  */
  int missing_required_options = 0;	

  args_info->help_given = 0;
  args_info->version_given = 0;
  args_info->rcfile_given = 0;
  args_info->verbose_given = 0;
  args_info->no_banner_given = 0;
  args_info->output_given = 0;
  args_info->compress_given = 0;
  args_info->hash_ngrams_given = 0;
  args_info->trie_depth_given = 0;
  args_info->trie_threshhold_given = 0;
  args_info->trie_theta_given = 0;
  args_info->use_classes_given = 0;
  args_info->use_flavors_given = 0;
  args_info->relax_given = 0;
  args_info->nlambdas_given = 0;
  args_info->wlambdas_given = 0;
  args_info->clambdas_given = 0;
  args_info->unknown_threshhold_given = 0;
  args_info->class_threshhold_given = 0;
  args_info->unknown_token_given = 0;
  args_info->unknown_tag_given = 0;
  args_info->eos_tag_given = 0;
  args_info->beam_width_given = 0;

  clear_args(args_info);

  /* rcfile handling */
  cmdline_parser_read_rcfile("/etc/mootcompilerc", args_info, 0);
  cmdline_parser_read_rcfile("~/.mootcompilerc", args_info, 0);
  /* end rcfile handling */

  optarg = 0;
  optind = 1;
  opterr = 1;
  optopt = '?';

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] = {
	{ "help", 0, NULL, 'h' },
	{ "version", 0, NULL, 'V' },
	{ "rcfile", 1, NULL, 'c' },
	{ "verbose", 1, NULL, 'v' },
	{ "no-banner", 0, NULL, 'B' },
	{ "output", 1, NULL, 'o' },
	{ "compress", 1, NULL, 'z' },
	{ "hash-ngrams", 1, NULL, 'g' },
	{ "trie-depth", 1, NULL, 'a' },
	{ "trie-threshhold", 1, NULL, 'A' },
	{ "trie-theta", 1, NULL, 0 },
	{ "use-classes", 1, NULL, 'L' },
	{ "use-flavors", 1, NULL, 'F' },
	{ "relax", 1, NULL, 'R' },
	{ "nlambdas", 1, NULL, 'N' },
	{ "wlambdas", 1, NULL, 'W' },
	{ "clambdas", 1, NULL, 'C' },
	{ "unknown-threshhold", 1, NULL, 't' },
	{ "class-threshhold", 1, NULL, 'T' },
	{ "unknown-token", 1, NULL, 'u' },
	{ "unknown-tag", 1, NULL, 'U' },
	{ "eos-tag", 1, NULL, 'e' },
	{ "beam-width", 1, NULL, 'Z' },
        { NULL,	0, NULL, 0 }
      };
      static char short_options[] = {
	'h',
	'V',
	'c', ':',
	'v', ':',
	'B',
	'o', ':',
	'z', ':',
	'g', ':',
	'a', ':',
	'A', ':',
	'L', ':',
	'F', ':',
	'R', ':',
	'N', ':',
	'W', ':',
	'C', ':',
	't', ':',
	'T', ':',
	'u', ':',
	'U', ':',
	'e', ':',
	'Z', ':',
	'\0'
      };

      c = getopt_long (argc, argv, short_options, long_options, &option_index);

      if (c == -1) break;	/* Exit from 'while (1)' loop.  */

      if (cmdline_parser_parse_option(c, long_options[option_index].name, optarg, args_info) != 0) {
	exit (EXIT_FAILURE);
      }
    } /* while */

  

  if ( missing_required_options )
    exit (EXIT_FAILURE);

  
  if (optind < argc) {
      int i = 0 ;
      args_info->inputs_num = argc - optind ;
      args_info->inputs = (char **)(malloc ((args_info->inputs_num)*sizeof(char *))) ;
      while (optind < argc)
        args_info->inputs[ i++ ] = gog_strdup (argv[optind++]) ; 
  }

  return 0;
}


/* Parse a single option */
int
cmdline_parser_parse_option(char oshort, const char *olong, const char *val,
			       struct gengetopt_args_info *args_info)
{
  if (!oshort && !(olong && *olong)) return 1;  /* ignore null options */

#ifdef cmdline_parser_DEBUG
  fprintf(stderr, "parse_option(): oshort='%c', olong='%s', val='%s'\n", oshort, olong, val);*/
#endif

  switch (oshort)
    {
      case 'h':	 /* Print help and exit. */
          if (args_info->help_given) {
            fprintf(stderr, "%s: `--help' (`-h') option given more than once\n", PROGRAM);
          }
          clear_args(args_info);
          cmdline_parser_print_help();
          exit(EXIT_SUCCESS);
        
          break;
        
        case 'V':	 /* Print version and exit. */
          if (args_info->version_given) {
            fprintf(stderr, "%s: `--version' (`-V') option given more than once\n", PROGRAM);
          }
          clear_args(args_info);
          cmdline_parser_print_version();
          exit(EXIT_SUCCESS);
        
          break;
        
        case 'c':	 /* Read an alternate configuration file. */
          if (args_info->rcfile_given) {
            fprintf(stderr, "%s: `--rcfile' (`-c') option given more than once\n", PROGRAM);
          }
          cmdline_parser_read_rcfile(val,args_info,1);
          break;
        
        case 'v':	 /* Verbosity level. */
          if (args_info->verbose_given) {
            fprintf(stderr, "%s: `--verbose' (`-v') option given more than once\n", PROGRAM);
          }
          args_info->verbose_given++;
          args_info->verbose_arg = (int)atoi(val);
          break;
        
        case 'B':	 /* Suppress initial banner message (implied at verbosity levels <= 2) */
          if (args_info->no_banner_given) {
            fprintf(stderr, "%s: `--no-banner' (`-B') option given more than once\n", PROGRAM);
          }
          args_info->no_banner_given++;
         if (args_info->no_banner_given <= 1)
           args_info->no_banner_flag = !(args_info->no_banner_flag);
          break;
        
        case 'o':	 /* Specify output file (default=stdout). */
          if (args_info->output_given) {
            fprintf(stderr, "%s: `--output' (`-o') option given more than once\n", PROGRAM);
          }
          args_info->output_given++;
          if (args_info->output_arg) free(args_info->output_arg);
          args_info->output_arg = gog_strdup(val);
          break;
        
        case 'z':	 /* Compression level for output file. */
          if (args_info->compress_given) {
            fprintf(stderr, "%s: `--compress' (`-z') option given more than once\n", PROGRAM);
          }
          args_info->compress_given++;
          args_info->compress_arg = (int)atoi(val);
          break;
        
        case 'g':	 /* Whether to hash stored n-grams (default=no) */
          if (args_info->hash_ngrams_given) {
            fprintf(stderr, "%s: `--hash-ngrams' (`-g') option given more than once\n", PROGRAM);
          }
          args_info->hash_ngrams_given++;
          args_info->hash_ngrams_arg = (int)atoi(val);
          break;
        
        case 'a':	 /* Maximum depth of suffix trie. */
          if (args_info->trie_depth_given) {
            fprintf(stderr, "%s: `--trie-depth' (`-a') option given more than once\n", PROGRAM);
          }
          args_info->trie_depth_given++;
          args_info->trie_depth_arg = (int)atoi(val);
          break;
        
        case 'A':	 /* Frequency upper bound for trie inclusion. */
          if (args_info->trie_threshhold_given) {
            fprintf(stderr, "%s: `--trie-threshhold' (`-A') option given more than once\n", PROGRAM);
          }
          args_info->trie_threshhold_given++;
          args_info->trie_threshhold_arg = (int)atoi(val);
          break;
        
        case 'L':	 /* Whether to use lexical class-probabilities. */
          if (args_info->use_classes_given) {
            fprintf(stderr, "%s: `--use-classes' (`-L') option given more than once\n", PROGRAM);
          }
          args_info->use_classes_given++;
          args_info->use_classes_arg = (int)atoi(val);
          break;
        
        case 'F':	 /* Whether to use token 'flavor' heuristics (default=1 (true)) */
          if (args_info->use_flavors_given) {
            fprintf(stderr, "%s: `--use-flavors' (`-F') option given more than once\n", PROGRAM);
          }
          args_info->use_flavors_given++;
          args_info->use_flavors_arg = (int)atoi(val);
          break;
        
        case 'R':	 /* Whether to relax token-tag associability (default=1 (true)) */
          if (args_info->relax_given) {
            fprintf(stderr, "%s: `--relax' (`-R') option given more than once\n", PROGRAM);
          }
          args_info->relax_given++;
          args_info->relax_arg = (int)atoi(val);
          break;
        
        case 'N':	 /* N-Gram smoothing constants (default=estimate) */
          if (args_info->nlambdas_given) {
            fprintf(stderr, "%s: `--nlambdas' (`-N') option given more than once\n", PROGRAM);
          }
          args_info->nlambdas_given++;
          if (args_info->nlambdas_arg) free(args_info->nlambdas_arg);
          args_info->nlambdas_arg = gog_strdup(val);
          break;
        
        case 'W':	 /* Lexical smoothing constants (default=estimate) */
          if (args_info->wlambdas_given) {
            fprintf(stderr, "%s: `--wlambdas' (`-W') option given more than once\n", PROGRAM);
          }
          args_info->wlambdas_given++;
          if (args_info->wlambdas_arg) free(args_info->wlambdas_arg);
          args_info->wlambdas_arg = gog_strdup(val);
          break;
        
        case 'C':	 /* Lexical-class smoothing constants (default=estimate) */
          if (args_info->clambdas_given) {
            fprintf(stderr, "%s: `--clambdas' (`-C') option given more than once\n", PROGRAM);
          }
          args_info->clambdas_given++;
          if (args_info->clambdas_arg) free(args_info->clambdas_arg);
          args_info->clambdas_arg = gog_strdup(val);
          break;
        
        case 't':	 /* Freq. threshhold for 'unknown' lexical probabilities */
          if (args_info->unknown_threshhold_given) {
            fprintf(stderr, "%s: `--unknown-threshhold' (`-t') option given more than once\n", PROGRAM);
          }
          args_info->unknown_threshhold_given++;
          args_info->unknown_threshhold_arg = (double)strtod(val, NULL);
          break;
        
        case 'T':	 /* Freq. threshhold for 'unknown' class probabilities */
          if (args_info->class_threshhold_given) {
            fprintf(stderr, "%s: `--class-threshhold' (`-T') option given more than once\n", PROGRAM);
          }
          args_info->class_threshhold_given++;
          args_info->class_threshhold_arg = (double)strtod(val, NULL);
          break;
        
        case 'u':	 /* Symbolic name of the 'unknown' token */
          if (args_info->unknown_token_given) {
            fprintf(stderr, "%s: `--unknown-token' (`-u') option given more than once\n", PROGRAM);
          }
          args_info->unknown_token_given++;
          if (args_info->unknown_token_arg) free(args_info->unknown_token_arg);
          args_info->unknown_token_arg = gog_strdup(val);
          break;
        
        case 'U':	 /* Symbolic name of the 'unknown' tag */
          if (args_info->unknown_tag_given) {
            fprintf(stderr, "%s: `--unknown-tag' (`-U') option given more than once\n", PROGRAM);
          }
          args_info->unknown_tag_given++;
          if (args_info->unknown_tag_arg) free(args_info->unknown_tag_arg);
          args_info->unknown_tag_arg = gog_strdup(val);
          break;
        
        case 'e':	 /* Specify boundary tag (default=__$) */
          if (args_info->eos_tag_given) {
            fprintf(stderr, "%s: `--eos-tag' (`-e') option given more than once\n", PROGRAM);
          }
          args_info->eos_tag_given++;
          if (args_info->eos_tag_arg) free(args_info->eos_tag_arg);
          args_info->eos_tag_arg = gog_strdup(val);
          break;
        
        case 'Z':	 /* Specify cutoff factor for beam pruning */
          if (args_info->beam_width_given) {
            fprintf(stderr, "%s: `--beam-width' (`-Z') option given more than once\n", PROGRAM);
          }
          args_info->beam_width_given++;
          args_info->beam_width_arg = (double)strtod(val, NULL);
          break;
        
        case 0:	 /* Long option(s) with no short form */
        /* Print help and exit. */
          if (strcmp(olong, "help") == 0) {
            if (args_info->help_given) {
              fprintf(stderr, "%s: `--help' (`-h') option given more than once\n", PROGRAM);
            }
            clear_args(args_info);
            cmdline_parser_print_help();
            exit(EXIT_SUCCESS);
          
          }
          
          /* Print version and exit. */
          else if (strcmp(olong, "version") == 0) {
            if (args_info->version_given) {
              fprintf(stderr, "%s: `--version' (`-V') option given more than once\n", PROGRAM);
            }
            clear_args(args_info);
            cmdline_parser_print_version();
            exit(EXIT_SUCCESS);
          
          }
          
          /* Read an alternate configuration file. */
          else if (strcmp(olong, "rcfile") == 0) {
            if (args_info->rcfile_given) {
              fprintf(stderr, "%s: `--rcfile' (`-c') option given more than once\n", PROGRAM);
            }
            cmdline_parser_read_rcfile(val,args_info,1);
          }
          
          /* Verbosity level. */
          else if (strcmp(olong, "verbose") == 0) {
            if (args_info->verbose_given) {
              fprintf(stderr, "%s: `--verbose' (`-v') option given more than once\n", PROGRAM);
            }
            args_info->verbose_given++;
            args_info->verbose_arg = (int)atoi(val);
          }
          
          /* Suppress initial banner message (implied at verbosity levels <= 2) */
          else if (strcmp(olong, "no-banner") == 0) {
            if (args_info->no_banner_given) {
              fprintf(stderr, "%s: `--no-banner' (`-B') option given more than once\n", PROGRAM);
            }
            args_info->no_banner_given++;
           if (args_info->no_banner_given <= 1)
             args_info->no_banner_flag = !(args_info->no_banner_flag);
          }
          
          /* Specify output file (default=stdout). */
          else if (strcmp(olong, "output") == 0) {
            if (args_info->output_given) {
              fprintf(stderr, "%s: `--output' (`-o') option given more than once\n", PROGRAM);
            }
            args_info->output_given++;
            if (args_info->output_arg) free(args_info->output_arg);
            args_info->output_arg = gog_strdup(val);
          }
          
          /* Compression level for output file. */
          else if (strcmp(olong, "compress") == 0) {
            if (args_info->compress_given) {
              fprintf(stderr, "%s: `--compress' (`-z') option given more than once\n", PROGRAM);
            }
            args_info->compress_given++;
            args_info->compress_arg = (int)atoi(val);
          }
          
          /* Whether to hash stored n-grams (default=no) */
          else if (strcmp(olong, "hash-ngrams") == 0) {
            if (args_info->hash_ngrams_given) {
              fprintf(stderr, "%s: `--hash-ngrams' (`-g') option given more than once\n", PROGRAM);
            }
            args_info->hash_ngrams_given++;
            args_info->hash_ngrams_arg = (int)atoi(val);
          }
          
          /* Maximum depth of suffix trie. */
          else if (strcmp(olong, "trie-depth") == 0) {
            if (args_info->trie_depth_given) {
              fprintf(stderr, "%s: `--trie-depth' (`-a') option given more than once\n", PROGRAM);
            }
            args_info->trie_depth_given++;
            args_info->trie_depth_arg = (int)atoi(val);
          }
          
          /* Frequency upper bound for trie inclusion. */
          else if (strcmp(olong, "trie-threshhold") == 0) {
            if (args_info->trie_threshhold_given) {
              fprintf(stderr, "%s: `--trie-threshhold' (`-A') option given more than once\n", PROGRAM);
            }
            args_info->trie_threshhold_given++;
            args_info->trie_threshhold_arg = (int)atoi(val);
          }
          
          /* Suffix backoff coefficient. */
          else if (strcmp(olong, "trie-theta") == 0) {
            if (args_info->trie_theta_given) {
              fprintf(stderr, "%s: `--trie-theta' option given more than once\n", PROGRAM);
            }
            args_info->trie_theta_given++;
            args_info->trie_theta_arg = (float)strtod(val, NULL);
          }
          
          /* Whether to use lexical class-probabilities. */
          else if (strcmp(olong, "use-classes") == 0) {
            if (args_info->use_classes_given) {
              fprintf(stderr, "%s: `--use-classes' (`-L') option given more than once\n", PROGRAM);
            }
            args_info->use_classes_given++;
            args_info->use_classes_arg = (int)atoi(val);
          }
          
          /* Whether to use token 'flavor' heuristics (default=1 (true)) */
          else if (strcmp(olong, "use-flavors") == 0) {
            if (args_info->use_flavors_given) {
              fprintf(stderr, "%s: `--use-flavors' (`-F') option given more than once\n", PROGRAM);
            }
            args_info->use_flavors_given++;
            args_info->use_flavors_arg = (int)atoi(val);
          }
          
          /* Whether to relax token-tag associability (default=1 (true)) */
          else if (strcmp(olong, "relax") == 0) {
            if (args_info->relax_given) {
              fprintf(stderr, "%s: `--relax' (`-R') option given more than once\n", PROGRAM);
            }
            args_info->relax_given++;
            args_info->relax_arg = (int)atoi(val);
          }
          
          /* N-Gram smoothing constants (default=estimate) */
          else if (strcmp(olong, "nlambdas") == 0) {
            if (args_info->nlambdas_given) {
              fprintf(stderr, "%s: `--nlambdas' (`-N') option given more than once\n", PROGRAM);
            }
            args_info->nlambdas_given++;
            if (args_info->nlambdas_arg) free(args_info->nlambdas_arg);
            args_info->nlambdas_arg = gog_strdup(val);
          }
          
          /* Lexical smoothing constants (default=estimate) */
          else if (strcmp(olong, "wlambdas") == 0) {
            if (args_info->wlambdas_given) {
              fprintf(stderr, "%s: `--wlambdas' (`-W') option given more than once\n", PROGRAM);
            }
            args_info->wlambdas_given++;
            if (args_info->wlambdas_arg) free(args_info->wlambdas_arg);
            args_info->wlambdas_arg = gog_strdup(val);
          }
          
          /* Lexical-class smoothing constants (default=estimate) */
          else if (strcmp(olong, "clambdas") == 0) {
            if (args_info->clambdas_given) {
              fprintf(stderr, "%s: `--clambdas' (`-C') option given more than once\n", PROGRAM);
            }
            args_info->clambdas_given++;
            if (args_info->clambdas_arg) free(args_info->clambdas_arg);
            args_info->clambdas_arg = gog_strdup(val);
          }
          
          /* Freq. threshhold for 'unknown' lexical probabilities */
          else if (strcmp(olong, "unknown-threshhold") == 0) {
            if (args_info->unknown_threshhold_given) {
              fprintf(stderr, "%s: `--unknown-threshhold' (`-t') option given more than once\n", PROGRAM);
            }
            args_info->unknown_threshhold_given++;
            args_info->unknown_threshhold_arg = (double)strtod(val, NULL);
          }
          
          /* Freq. threshhold for 'unknown' class probabilities */
          else if (strcmp(olong, "class-threshhold") == 0) {
            if (args_info->class_threshhold_given) {
              fprintf(stderr, "%s: `--class-threshhold' (`-T') option given more than once\n", PROGRAM);
            }
            args_info->class_threshhold_given++;
            args_info->class_threshhold_arg = (double)strtod(val, NULL);
          }
          
          /* Symbolic name of the 'unknown' token */
          else if (strcmp(olong, "unknown-token") == 0) {
            if (args_info->unknown_token_given) {
              fprintf(stderr, "%s: `--unknown-token' (`-u') option given more than once\n", PROGRAM);
            }
            args_info->unknown_token_given++;
            if (args_info->unknown_token_arg) free(args_info->unknown_token_arg);
            args_info->unknown_token_arg = gog_strdup(val);
          }
          
          /* Symbolic name of the 'unknown' tag */
          else if (strcmp(olong, "unknown-tag") == 0) {
            if (args_info->unknown_tag_given) {
              fprintf(stderr, "%s: `--unknown-tag' (`-U') option given more than once\n", PROGRAM);
            }
            args_info->unknown_tag_given++;
            if (args_info->unknown_tag_arg) free(args_info->unknown_tag_arg);
            args_info->unknown_tag_arg = gog_strdup(val);
          }
          
          /* Specify boundary tag (default=__$) */
          else if (strcmp(olong, "eos-tag") == 0) {
            if (args_info->eos_tag_given) {
              fprintf(stderr, "%s: `--eos-tag' (`-e') option given more than once\n", PROGRAM);
            }
            args_info->eos_tag_given++;
            if (args_info->eos_tag_arg) free(args_info->eos_tag_arg);
            args_info->eos_tag_arg = gog_strdup(val);
          }
          
          /* Specify cutoff factor for beam pruning */
          else if (strcmp(olong, "beam-width") == 0) {
            if (args_info->beam_width_given) {
              fprintf(stderr, "%s: `--beam-width' (`-Z') option given more than once\n", PROGRAM);
            }
            args_info->beam_width_given++;
            args_info->beam_width_arg = (double)strtod(val, NULL);
          }
          
          else {
            fprintf(stderr, "%s: unknown long option '%s'.\n", PROGRAM, olong);
            return (EXIT_FAILURE);
          }
          break;

        case '?':	 /* Invalid Option */
          fprintf(stderr, "%s: unknown option '%s'.\n", PROGRAM, olong);
          return (EXIT_FAILURE);


        default:	/* bug: options not considered.  */
          fprintf (stderr, "%s: option unknown: %c\n", PROGRAM, oshort);
          abort ();
        } /* switch */
  return 0;
}


/* Initialize options not yet given from environmental defaults */
void
cmdline_parser_envdefaults(struct gengetopt_args_info *args_info)
{
  

  return;
}


/* Load option values from an .rc file */
void
cmdline_parser_read_rcfile(const char *filename,
			      struct gengetopt_args_info *args_info,
			      int user_specified)
{
  char *fullname;
  FILE *rcfile;

  if (!filename) return; /* ignore NULL filenames */

#if defined(HAVE_GETUID) && defined(HAVE_GETPWUID)
  if (*filename == '~') {
    /* tilde-expansion hack */
    struct passwd *pwent = getpwuid(getuid());
    if (!pwent) {
      fprintf(stderr, "%s: user-id %d not found!\n", PROGRAM, getuid());
      return;
    }
    if (!pwent->pw_dir) {
      fprintf(stderr, "%s: home directory for user-id %d not found!\n", PROGRAM, getuid());
      return;
    }
    fullname = (char *)malloc(strlen(pwent->pw_dir)+strlen(filename));
    strcpy(fullname, pwent->pw_dir);
    strcat(fullname, filename+1);
  } else {
    fullname = gog_strdup(filename);
  }
#else /* !(defined(HAVE_GETUID) && defined(HAVE_GETPWUID)) */
  fullname = gog_strdup(filename);
#endif /* defined(HAVE_GETUID) && defined(HAVE_GETPWUID) */

  /* try to open */
  rcfile = fopen(fullname,"r");
  if (!rcfile) {
    if (user_specified) {
      fprintf(stderr, "%s: warning: open failed for rc-file '%s': %s\n",
	      PROGRAM, fullname, strerror(errno));
    }
  }
  else {
   cmdline_parser_read_rc_stream(rcfile, fullname, args_info);
  }

  /* cleanup */
  if (fullname != filename) free(fullname);
  if (rcfile) fclose(rcfile);

  return;
}


/* Parse option values from an .rc file : guts */
#define OPTPARSE_GET 32
void
cmdline_parser_read_rc_stream(FILE *rcfile,
				 const char *filename,
				 struct gengetopt_args_info *args_info)
{
  char *optname  = (char *)malloc(OPTPARSE_GET);
  char *optval   = (char *)malloc(OPTPARSE_GET);
  size_t onsize  = OPTPARSE_GET;
  size_t ovsize  = OPTPARSE_GET;
  size_t onlen   = 0;
  size_t ovlen   = 0;
  int    lineno  = 0;
  char c;

#ifdef cmdline_parser_DEBUG
  fprintf(stderr, "cmdline_parser_read_rc_stream('%s'):\n", filename);
#endif

  while ((c = fgetc(rcfile)) != EOF) {
    onlen = 0;
    ovlen = 0;
    lineno++;

    /* -- get next option-name */
    /* skip leading space and comments */
    if (isspace(c)) continue;
    if (c == '#') {
      while ((c = fgetc(rcfile)) != EOF) {
	if (c == '\n') break;
      }
      continue;
    }

    /* parse option-name */
    while (c != EOF && c != '=' && !isspace(c)) {
      /* re-allocate if necessary */
      if (onlen >= onsize-1) {
	char *tmp = (char *)malloc(onsize+OPTPARSE_GET);
	strcpy(tmp,optname);
	free(optname);

	onsize += OPTPARSE_GET;
	optname = tmp;
      }
      optname[onlen++] = c;
      c = fgetc(rcfile);
    }
    optname[onlen++] = '\0';

#ifdef cmdline_parser_DEBUG
    fprintf(stderr, "cmdline_parser_read_rc_stream('%s'): line %d: optname='%s'\n",
	    filename, lineno, optname);
#endif

    /* -- get next option-value */
    /* skip leading space */
    while ((c = fgetc(rcfile)) != EOF && isspace(c)) {
      ;
    }

    /* parse option-value */
    while (c != EOF && c != '\n') {
      /* re-allocate if necessary */
      if (ovlen >= ovsize-1) {
	char *tmp = (char *)malloc(ovsize+OPTPARSE_GET);
	strcpy(tmp,optval);
	free(optval);
	ovsize += OPTPARSE_GET;
	optval = tmp;
      }
      optval[ovlen++] = c;
      c = fgetc(rcfile);
    }
    optval[ovlen++] = '\0';

    /* now do the action for the option */
    if (cmdline_parser_parse_option('\0',optname,optval,args_info) != 0) {
      fprintf(stderr, "%s: error in file '%s' at line %d.\n", PROGRAM, filename, lineno);
      
    }
  }

  /* cleanup */
  free(optname);
  free(optval);

  return;
}
