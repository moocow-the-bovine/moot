/* -*- Mode: C -*-
 *
 * File: waste_cmdparser.c
 * Description: Code for command-line parser struct gengetopt_args_info.
 *
 * File autogenerated by optgen.perl version 0.07
 * generated with the following command:
 * /usr/local/bin/optgen.perl -u -l --nopod -F waste_cmdparser waste.gog
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
# define PROGRAM "waste"
#endif

/* #define cmdline_parser_DEBUG */

/* Check for "configure's" getopt check result.  */
#ifndef HAVE_GETOPT_LONG
# include "getopt.h"
#else
# include <getopt.h>
#endif

#include "waste_cmdparser.h"


/* user code section */

/* end user  code section */


void
cmdline_parser_print_version (void)
{
  printf("waste (%s %s) by Bryan Jurish <moocow@cpan.org> and Kay-Michael Würzner <wuerzner@bbaw.de>\n", PACKAGE, VERSION);
}

void
cmdline_parser_print_help (void)
{
  cmdline_parser_print_version ();
  printf("\n");
  printf("Purpose:\n");
  printf("  Word- and Sentence-Token Extractor using a Hidden Markov Model\n");
  printf("\n");
  
  printf("Usage: %s [OPTIONS]... FILE(s)\n", "waste");
  
  printf("\n");
  printf(" Arguments:\n");
  printf("   FILE(s)  Input files\n");
  
  printf("\n");
  printf(" Options:\n");
  printf("   -h        --help                  Print help and exit.\n");
  printf("   -V        --version               Print version and exit.\n");
  printf("   -cFILE    --rcfile=FILE           Read an alternate configuration file.\n");
  printf("   -vLEVEL   --verbose=LEVEL         Verbosity level.\n");
  printf("   -B        --no-banner             Suppress initial banner message (implied at verbosity levels <= 2)\n");
  printf("   -l        --list                  Arguments are input-file lists.\n");
  printf("   -r        --recover               Attempt to recover from minor errors.\n");
  printf("   -oFILE    --output=FILE           Write output to FILE.\n");
  printf("\n");
  printf(" Mode Options:\n");
  printf("   -f        --full                  Alias for --scan --lex --tag --decode (default)\n");
  printf("             --train                 Training mode (similar to --lex)\n");
  printf("   -s        --scan                  Enable raw text scanning stage.\n");
  printf("   -S        --no-scan               Disable raw text scanning stage.\n");
  printf("   -x        --lex                   Enable lexical classification stage.\n");
  printf("   -X        --no-lex                Disable lexical classification stage.\n");
  printf("   -t        --tag                   Enable HMM Viterbi tagging stage.\n");
  printf("   -T        --no-tag                Disable HMM Viterbi tagging stage.\n");
  printf("   -d        --decode                Enable post-Viterbi decoding stage.\n");
  printf("   -D        --no-decode             Disable post-Viterbi decoding stage.\n");
  printf("\n");
  printf(" Lexer Options:\n");
  printf("   -aFILE    --abbrevs=FILE          Load abbreviation lexicon from FILE (1 word/line)\n");
  printf("   -jFILE    --conjunctions=FILE     Load conjunction lexicon from FILE (1 word/line)\n");
  printf("   -wFILE    --stopwords=FILE        Load stopword lexicon from FILE (1 word/line)\n");
  printf("   -y        --dehyphenate           Enable automatic dehyphenation in lexer (default)\n");
  printf("   -Y        --no-dehyphenate        Disable automatic dehyphenation in lexer.\n");
  printf("\n");
  printf(" HMM Options:\n");
  printf("   -MMODEL   --model=MODEL           Use HMM tokenizer model MODEL.\n");
  printf("\n");
  printf(" Format Options:\n");
  printf("   -IFORMAT  --input-format=FORMAT   Specify input or --scan mode format\n");
  printf("   -OFORMAT  --output-format=FORMAT  Specify output file format.\n");
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
  args_info->verbose_arg = 3; 
  args_info->no_banner_flag = 0; 
  args_info->list_flag = 0; 
  args_info->recover_flag = 0; 
  args_info->output_arg = gog_strdup("-"); 
  args_info->full_flag = 0; 
  args_info->train_flag = 0; 
  args_info->scan_flag = 0; 
  args_info->no_scan_flag = 0; 
  args_info->lex_flag = 0; 
  args_info->no_lex_flag = 0; 
  args_info->tag_flag = 0; 
  args_info->no_tag_flag = 0; 
  args_info->decode_flag = 0; 
  args_info->no_decode_flag = 0; 
  args_info->abbrevs_arg = NULL; 
  args_info->conjunctions_arg = NULL; 
  args_info->stopwords_arg = NULL; 
  args_info->dehyphenate_flag = 1; 
  args_info->no_dehyphenate_flag = 0; 
  args_info->model_arg = gog_strdup("waste.hmm"); 
  args_info->input_format_arg = NULL; 
  args_info->output_format_arg = NULL; 
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
  args_info->list_given = 0;
  args_info->recover_given = 0;
  args_info->output_given = 0;
  args_info->full_given = 0;
  args_info->train_given = 0;
  args_info->scan_given = 0;
  args_info->no_scan_given = 0;
  args_info->lex_given = 0;
  args_info->no_lex_given = 0;
  args_info->tag_given = 0;
  args_info->no_tag_given = 0;
  args_info->decode_given = 0;
  args_info->no_decode_given = 0;
  args_info->abbrevs_given = 0;
  args_info->conjunctions_given = 0;
  args_info->stopwords_given = 0;
  args_info->dehyphenate_given = 0;
  args_info->no_dehyphenate_given = 0;
  args_info->model_given = 0;
  args_info->input_format_given = 0;
  args_info->output_format_given = 0;

  clear_args(args_info);

  /* rcfile handling */
  
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
	{ "list", 0, NULL, 'l' },
	{ "recover", 0, NULL, 'r' },
	{ "output", 1, NULL, 'o' },
	{ "full", 0, NULL, 'f' },
	{ "train", 0, NULL, 0 },
	{ "scan", 0, NULL, 's' },
	{ "no-scan", 0, NULL, 'S' },
	{ "lex", 0, NULL, 'x' },
	{ "no-lex", 0, NULL, 'X' },
	{ "tag", 0, NULL, 't' },
	{ "no-tag", 0, NULL, 'T' },
	{ "decode", 0, NULL, 'd' },
	{ "no-decode", 0, NULL, 'D' },
	{ "abbrevs", 1, NULL, 'a' },
	{ "conjunctions", 1, NULL, 'j' },
	{ "stopwords", 1, NULL, 'w' },
	{ "dehyphenate", 0, NULL, 'y' },
	{ "no-dehyphenate", 0, NULL, 'Y' },
	{ "model", 1, NULL, 'M' },
	{ "input-format", 1, NULL, 'I' },
	{ "output-format", 1, NULL, 'O' },
        { NULL,	0, NULL, 0 }
      };
      static char short_options[] = {
	'h',
	'V',
	'c', ':',
	'v', ':',
	'B',
	'l',
	'r',
	'o', ':',
	'f',
	's',
	'S',
	'x',
	'X',
	't',
	'T',
	'd',
	'D',
	'a', ':',
	'j', ':',
	'w', ':',
	'y',
	'Y',
	'M', ':',
	'I', ':',
	'O', ':',
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
        
        case 'l':	 /* Arguments are input-file lists. */
          if (args_info->list_given) {
            fprintf(stderr, "%s: `--list' (`-l') option given more than once\n", PROGRAM);
          }
          args_info->list_given++;
         if (args_info->list_given <= 1)
           args_info->list_flag = !(args_info->list_flag);
          break;
        
        case 'r':	 /* Attempt to recover from minor errors. */
          if (args_info->recover_given) {
            fprintf(stderr, "%s: `--recover' (`-r') option given more than once\n", PROGRAM);
          }
          args_info->recover_given++;
         if (args_info->recover_given <= 1)
           args_info->recover_flag = !(args_info->recover_flag);
          break;
        
        case 'o':	 /* Write output to FILE. */
          if (args_info->output_given) {
            fprintf(stderr, "%s: `--output' (`-o') option given more than once\n", PROGRAM);
          }
          args_info->output_given++;
          if (args_info->output_arg) free(args_info->output_arg);
          args_info->output_arg = gog_strdup(val);
          break;
        
        case 'f':	 /* Alias for --scan --lex --tag --decode (default) */
          if (args_info->full_given) {
            fprintf(stderr, "%s: `--full' (`-f') option given more than once\n", PROGRAM);
          }
          args_info->full_given++;
         if (args_info->full_given <= 1)
           args_info->full_flag = !(args_info->full_flag);
          /* user code */
          args_info->full_flag = args_info->scan_flag = args_info->lex_flag = args_info->tag_flag = args_info->decode_flag = 1;
          break;
        
        case 's':	 /* Enable raw text scanning stage. */
          if (args_info->scan_given) {
            fprintf(stderr, "%s: `--scan' (`-s') option given more than once\n", PROGRAM);
          }
          args_info->scan_given++;
         if (args_info->scan_given <= 1)
           args_info->scan_flag = !(args_info->scan_flag);
          /* user code */
          args_info->scan_flag=1;
          break;
        
        case 'S':	 /* Disable raw text scanning stage. */
          if (args_info->no_scan_given) {
            fprintf(stderr, "%s: `--no-scan' (`-S') option given more than once\n", PROGRAM);
          }
          args_info->no_scan_given++;
         if (args_info->no_scan_given <= 1)
           args_info->no_scan_flag = !(args_info->no_scan_flag);
          /* user code */
          args_info->scan_flag=0;
          break;
        
        case 'x':	 /* Enable lexical classification stage. */
          if (args_info->lex_given) {
            fprintf(stderr, "%s: `--lex' (`-x') option given more than once\n", PROGRAM);
          }
          args_info->lex_given++;
         if (args_info->lex_given <= 1)
           args_info->lex_flag = !(args_info->lex_flag);
          /* user code */
          args_info->lex_flag=1;
          break;
        
        case 'X':	 /* Disable lexical classification stage. */
          if (args_info->no_lex_given) {
            fprintf(stderr, "%s: `--no-lex' (`-X') option given more than once\n", PROGRAM);
          }
          args_info->no_lex_given++;
         if (args_info->no_lex_given <= 1)
           args_info->no_lex_flag = !(args_info->no_lex_flag);
          /* user code */
          args_info->lex_flag=0;
          break;
        
        case 't':	 /* Enable HMM Viterbi tagging stage. */
          if (args_info->tag_given) {
            fprintf(stderr, "%s: `--tag' (`-t') option given more than once\n", PROGRAM);
          }
          args_info->tag_given++;
         if (args_info->tag_given <= 1)
           args_info->tag_flag = !(args_info->tag_flag);
          /* user code */
          args_info->tag_flag=1;
          break;
        
        case 'T':	 /* Disable HMM Viterbi tagging stage. */
          if (args_info->no_tag_given) {
            fprintf(stderr, "%s: `--no-tag' (`-T') option given more than once\n", PROGRAM);
          }
          args_info->no_tag_given++;
         if (args_info->no_tag_given <= 1)
           args_info->no_tag_flag = !(args_info->no_tag_flag);
          /* user code */
          args_info->tag_flag=0;
          break;
        
        case 'd':	 /* Enable post-Viterbi decoding stage. */
          if (args_info->decode_given) {
            fprintf(stderr, "%s: `--decode' (`-d') option given more than once\n", PROGRAM);
          }
          args_info->decode_given++;
         if (args_info->decode_given <= 1)
           args_info->decode_flag = !(args_info->decode_flag);
          /* user code */
          args_info->decode_flag=1;
          break;
        
        case 'D':	 /* Disable post-Viterbi decoding stage. */
          if (args_info->no_decode_given) {
            fprintf(stderr, "%s: `--no-decode' (`-D') option given more than once\n", PROGRAM);
          }
          args_info->no_decode_given++;
         if (args_info->no_decode_given <= 1)
           args_info->no_decode_flag = !(args_info->no_decode_flag);
          /* user code */
          args_info->decode_flag=0;
          break;
        
        case 'a':	 /* Load abbreviation lexicon from FILE (1 word/line) */
          if (args_info->abbrevs_given) {
            fprintf(stderr, "%s: `--abbrevs' (`-a') option given more than once\n", PROGRAM);
          }
          args_info->abbrevs_given++;
          if (args_info->abbrevs_arg) free(args_info->abbrevs_arg);
          args_info->abbrevs_arg = gog_strdup(val);
          break;
        
        case 'j':	 /* Load conjunction lexicon from FILE (1 word/line) */
          if (args_info->conjunctions_given) {
            fprintf(stderr, "%s: `--conjunctions' (`-j') option given more than once\n", PROGRAM);
          }
          args_info->conjunctions_given++;
          if (args_info->conjunctions_arg) free(args_info->conjunctions_arg);
          args_info->conjunctions_arg = gog_strdup(val);
          break;
        
        case 'w':	 /* Load stopword lexicon from FILE (1 word/line) */
          if (args_info->stopwords_given) {
            fprintf(stderr, "%s: `--stopwords' (`-w') option given more than once\n", PROGRAM);
          }
          args_info->stopwords_given++;
          if (args_info->stopwords_arg) free(args_info->stopwords_arg);
          args_info->stopwords_arg = gog_strdup(val);
          break;
        
        case 'y':	 /* Enable automatic dehyphenation in lexer (default) */
          if (args_info->dehyphenate_given) {
            fprintf(stderr, "%s: `--dehyphenate' (`-y') option given more than once\n", PROGRAM);
          }
          args_info->dehyphenate_given++;
         if (args_info->dehyphenate_given <= 1)
           args_info->dehyphenate_flag = !(args_info->dehyphenate_flag);
          /* user code */
          args_info->dehyphenate_flag=1;
          break;
        
        case 'Y':	 /* Disable automatic dehyphenation in lexer. */
          if (args_info->no_dehyphenate_given) {
            fprintf(stderr, "%s: `--no-dehyphenate' (`-Y') option given more than once\n", PROGRAM);
          }
          args_info->no_dehyphenate_given++;
         if (args_info->no_dehyphenate_given <= 1)
           args_info->no_dehyphenate_flag = !(args_info->no_dehyphenate_flag);
          /* user code */
          args_info->dehyphenate_flag=0;
          break;
        
        case 'M':	 /* Use HMM tokenizer model MODEL. */
          if (args_info->model_given) {
            fprintf(stderr, "%s: `--model' (`-M') option given more than once\n", PROGRAM);
          }
          args_info->model_given++;
          if (args_info->model_arg) free(args_info->model_arg);
          args_info->model_arg = gog_strdup(val);
          break;
        
        case 'I':	 /* Specify input or --scan mode format */
          if (args_info->input_format_given) {
            fprintf(stderr, "%s: `--input-format' (`-I') option given more than once\n", PROGRAM);
          }
          args_info->input_format_given++;
          if (args_info->input_format_arg) free(args_info->input_format_arg);
          args_info->input_format_arg = gog_strdup(val);
          break;
        
        case 'O':	 /* Specify output file format. */
          if (args_info->output_format_given) {
            fprintf(stderr, "%s: `--output-format' (`-O') option given more than once\n", PROGRAM);
          }
          args_info->output_format_given++;
          if (args_info->output_format_arg) free(args_info->output_format_arg);
          args_info->output_format_arg = gog_strdup(val);
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
          
          /* Arguments are input-file lists. */
          else if (strcmp(olong, "list") == 0) {
            if (args_info->list_given) {
              fprintf(stderr, "%s: `--list' (`-l') option given more than once\n", PROGRAM);
            }
            args_info->list_given++;
           if (args_info->list_given <= 1)
             args_info->list_flag = !(args_info->list_flag);
          }
          
          /* Attempt to recover from minor errors. */
          else if (strcmp(olong, "recover") == 0) {
            if (args_info->recover_given) {
              fprintf(stderr, "%s: `--recover' (`-r') option given more than once\n", PROGRAM);
            }
            args_info->recover_given++;
           if (args_info->recover_given <= 1)
             args_info->recover_flag = !(args_info->recover_flag);
          }
          
          /* Write output to FILE. */
          else if (strcmp(olong, "output") == 0) {
            if (args_info->output_given) {
              fprintf(stderr, "%s: `--output' (`-o') option given more than once\n", PROGRAM);
            }
            args_info->output_given++;
            if (args_info->output_arg) free(args_info->output_arg);
            args_info->output_arg = gog_strdup(val);
          }
          
          /* Alias for --scan --lex --tag --decode (default) */
          else if (strcmp(olong, "full") == 0) {
            if (args_info->full_given) {
              fprintf(stderr, "%s: `--full' (`-f') option given more than once\n", PROGRAM);
            }
            args_info->full_given++;
           if (args_info->full_given <= 1)
             args_info->full_flag = !(args_info->full_flag);
            /* user code */
            args_info->full_flag = args_info->scan_flag = args_info->lex_flag = args_info->tag_flag = args_info->decode_flag = 1;
          }
          
          /* Training mode (similar to --lex) */
          else if (strcmp(olong, "train") == 0) {
            if (args_info->train_given) {
              fprintf(stderr, "%s: `--train' option given more than once\n", PROGRAM);
            }
            args_info->train_given++;
           if (args_info->train_given <= 1)
             args_info->train_flag = !(args_info->train_flag);
          }
          
          /* Enable raw text scanning stage. */
          else if (strcmp(olong, "scan") == 0) {
            if (args_info->scan_given) {
              fprintf(stderr, "%s: `--scan' (`-s') option given more than once\n", PROGRAM);
            }
            args_info->scan_given++;
           if (args_info->scan_given <= 1)
             args_info->scan_flag = !(args_info->scan_flag);
            /* user code */
            args_info->scan_flag=1;
          }
          
          /* Disable raw text scanning stage. */
          else if (strcmp(olong, "no-scan") == 0) {
            if (args_info->no_scan_given) {
              fprintf(stderr, "%s: `--no-scan' (`-S') option given more than once\n", PROGRAM);
            }
            args_info->no_scan_given++;
           if (args_info->no_scan_given <= 1)
             args_info->no_scan_flag = !(args_info->no_scan_flag);
            /* user code */
            args_info->scan_flag=0;
          }
          
          /* Enable lexical classification stage. */
          else if (strcmp(olong, "lex") == 0) {
            if (args_info->lex_given) {
              fprintf(stderr, "%s: `--lex' (`-x') option given more than once\n", PROGRAM);
            }
            args_info->lex_given++;
           if (args_info->lex_given <= 1)
             args_info->lex_flag = !(args_info->lex_flag);
            /* user code */
            args_info->lex_flag=1;
          }
          
          /* Disable lexical classification stage. */
          else if (strcmp(olong, "no-lex") == 0) {
            if (args_info->no_lex_given) {
              fprintf(stderr, "%s: `--no-lex' (`-X') option given more than once\n", PROGRAM);
            }
            args_info->no_lex_given++;
           if (args_info->no_lex_given <= 1)
             args_info->no_lex_flag = !(args_info->no_lex_flag);
            /* user code */
            args_info->lex_flag=0;
          }
          
          /* Enable HMM Viterbi tagging stage. */
          else if (strcmp(olong, "tag") == 0) {
            if (args_info->tag_given) {
              fprintf(stderr, "%s: `--tag' (`-t') option given more than once\n", PROGRAM);
            }
            args_info->tag_given++;
           if (args_info->tag_given <= 1)
             args_info->tag_flag = !(args_info->tag_flag);
            /* user code */
            args_info->tag_flag=1;
          }
          
          /* Disable HMM Viterbi tagging stage. */
          else if (strcmp(olong, "no-tag") == 0) {
            if (args_info->no_tag_given) {
              fprintf(stderr, "%s: `--no-tag' (`-T') option given more than once\n", PROGRAM);
            }
            args_info->no_tag_given++;
           if (args_info->no_tag_given <= 1)
             args_info->no_tag_flag = !(args_info->no_tag_flag);
            /* user code */
            args_info->tag_flag=0;
          }
          
          /* Enable post-Viterbi decoding stage. */
          else if (strcmp(olong, "decode") == 0) {
            if (args_info->decode_given) {
              fprintf(stderr, "%s: `--decode' (`-d') option given more than once\n", PROGRAM);
            }
            args_info->decode_given++;
           if (args_info->decode_given <= 1)
             args_info->decode_flag = !(args_info->decode_flag);
            /* user code */
            args_info->decode_flag=1;
          }
          
          /* Disable post-Viterbi decoding stage. */
          else if (strcmp(olong, "no-decode") == 0) {
            if (args_info->no_decode_given) {
              fprintf(stderr, "%s: `--no-decode' (`-D') option given more than once\n", PROGRAM);
            }
            args_info->no_decode_given++;
           if (args_info->no_decode_given <= 1)
             args_info->no_decode_flag = !(args_info->no_decode_flag);
            /* user code */
            args_info->decode_flag=0;
          }
          
          /* Load abbreviation lexicon from FILE (1 word/line) */
          else if (strcmp(olong, "abbrevs") == 0) {
            if (args_info->abbrevs_given) {
              fprintf(stderr, "%s: `--abbrevs' (`-a') option given more than once\n", PROGRAM);
            }
            args_info->abbrevs_given++;
            if (args_info->abbrevs_arg) free(args_info->abbrevs_arg);
            args_info->abbrevs_arg = gog_strdup(val);
          }
          
          /* Load conjunction lexicon from FILE (1 word/line) */
          else if (strcmp(olong, "conjunctions") == 0) {
            if (args_info->conjunctions_given) {
              fprintf(stderr, "%s: `--conjunctions' (`-j') option given more than once\n", PROGRAM);
            }
            args_info->conjunctions_given++;
            if (args_info->conjunctions_arg) free(args_info->conjunctions_arg);
            args_info->conjunctions_arg = gog_strdup(val);
          }
          
          /* Load stopword lexicon from FILE (1 word/line) */
          else if (strcmp(olong, "stopwords") == 0) {
            if (args_info->stopwords_given) {
              fprintf(stderr, "%s: `--stopwords' (`-w') option given more than once\n", PROGRAM);
            }
            args_info->stopwords_given++;
            if (args_info->stopwords_arg) free(args_info->stopwords_arg);
            args_info->stopwords_arg = gog_strdup(val);
          }
          
          /* Enable automatic dehyphenation in lexer (default) */
          else if (strcmp(olong, "dehyphenate") == 0) {
            if (args_info->dehyphenate_given) {
              fprintf(stderr, "%s: `--dehyphenate' (`-y') option given more than once\n", PROGRAM);
            }
            args_info->dehyphenate_given++;
           if (args_info->dehyphenate_given <= 1)
             args_info->dehyphenate_flag = !(args_info->dehyphenate_flag);
            /* user code */
            args_info->dehyphenate_flag=1;
          }
          
          /* Disable automatic dehyphenation in lexer. */
          else if (strcmp(olong, "no-dehyphenate") == 0) {
            if (args_info->no_dehyphenate_given) {
              fprintf(stderr, "%s: `--no-dehyphenate' (`-Y') option given more than once\n", PROGRAM);
            }
            args_info->no_dehyphenate_given++;
           if (args_info->no_dehyphenate_given <= 1)
             args_info->no_dehyphenate_flag = !(args_info->no_dehyphenate_flag);
            /* user code */
            args_info->dehyphenate_flag=0;
          }
          
          /* Use HMM tokenizer model MODEL. */
          else if (strcmp(olong, "model") == 0) {
            if (args_info->model_given) {
              fprintf(stderr, "%s: `--model' (`-M') option given more than once\n", PROGRAM);
            }
            args_info->model_given++;
            if (args_info->model_arg) free(args_info->model_arg);
            args_info->model_arg = gog_strdup(val);
          }
          
          /* Specify input or --scan mode format */
          else if (strcmp(olong, "input-format") == 0) {
            if (args_info->input_format_given) {
              fprintf(stderr, "%s: `--input-format' (`-I') option given more than once\n", PROGRAM);
            }
            args_info->input_format_given++;
            if (args_info->input_format_arg) free(args_info->input_format_arg);
            args_info->input_format_arg = gog_strdup(val);
          }
          
          /* Specify output file format. */
          else if (strcmp(olong, "output-format") == 0) {
            if (args_info->output_format_given) {
              fprintf(stderr, "%s: `--output-format' (`-O') option given more than once\n", PROGRAM);
            }
            args_info->output_format_given++;
            if (args_info->output_format_arg) free(args_info->output_format_arg);
            args_info->output_format_arg = gog_strdup(val);
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
