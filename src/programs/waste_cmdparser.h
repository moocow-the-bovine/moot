/* -*- Mode: C -*-
 *
 * File: waste_cmdparser.h
 * Description: Headers for command-line parser struct gengetopt_args_info.
 *
 * File autogenerated by optgen.perl version 0.07.
 *
 */

#ifndef waste_cmdparser_h
#define waste_cmdparser_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * moocow: Never set PACKAGE and VERSION here.
 */

struct gengetopt_args_info {
  char * rcfile_arg;	 /* Read an alternate configuration file. (default=NULL). */
  int verbose_arg;	 /* Verbosity level. (default=3). */
  int no_banner_flag;	 /* Suppress initial banner message (implied at verbosity levels <= 2) (default=0). */
  int list_flag;	 /* Arguments are input-file lists. (default=0). */
  int recover_flag;	 /* Attempt to recover from minor errors. (default=0). */
  char * output_arg;	 /* Write output to FILE. (default=-). */
  int full_flag;	 /* Alias for --scan --lex --tag --decode --annotate (default) (default=0). */
  int train_flag;	 /* Training mode (similar to --lex) (default=0). */
  int scan_flag;	 /* Enable raw text scanning stage. (default=0). */
  int no_scan_flag;	 /* Disable raw text scanning stage. (default=0). */
  int lex_flag;	 /* Enable lexical classification stage. (default=0). */
  int no_lex_flag;	 /* Disable lexical classification stage. (default=0). */
  int tag_flag;	 /* Enable HMM Viterbi tagging stage. (default=0). */
  int no_tag_flag;	 /* Disable HMM Viterbi tagging stage. (default=0). */
  int decode_flag;	 /* Enable post-Viterbi decoding stage. (default=0). */
  int no_decode_flag;	 /* Disable post-Viterbi decoding stage. (default=0). */
  int annotate_flag;	 /* Enable text-based annotation stage. (default=0). */
  int no_annotate_flag;	 /* Disable text-based annotation stage. (default=0). */
  char * abbrevs_arg;	 /* Load abbreviation lexicon from FILE (1 word/line) (default=NULL). */
  char * conjunctions_arg;	 /* Load conjunction lexicon from FILE (1 word/line) (default=NULL). */
  char * stopwords_arg;	 /* Load stopword lexicon from FILE (1 word/line) (default=NULL). */
  int dehyphenate_flag;	 /* Enable automatic dehyphenation in lexer (default) (default=1). */
  int no_dehyphenate_flag;	 /* Disable automatic dehyphenation in lexer. (default=0). */
  char * model_arg;	 /* Use HMM tokenizer model MODEL. (default=waste.hmm). */
  char * input_format_arg;	 /* Specify input or --scan mode format (default=NULL). */
  char * output_format_arg;	 /* Specify output file format. (default=NULL). */

  int help_given;	 /* Whether help was given */
  int version_given;	 /* Whether version was given */
  int rcfile_given;	 /* Whether rcfile was given */
  int verbose_given;	 /* Whether verbose was given */
  int no_banner_given;	 /* Whether no-banner was given */
  int list_given;	 /* Whether list was given */
  int recover_given;	 /* Whether recover was given */
  int output_given;	 /* Whether output was given */
  int full_given;	 /* Whether full was given */
  int train_given;	 /* Whether train was given */
  int scan_given;	 /* Whether scan was given */
  int no_scan_given;	 /* Whether no-scan was given */
  int lex_given;	 /* Whether lex was given */
  int no_lex_given;	 /* Whether no-lex was given */
  int tag_given;	 /* Whether tag was given */
  int no_tag_given;	 /* Whether no-tag was given */
  int decode_given;	 /* Whether decode was given */
  int no_decode_given;	 /* Whether no-decode was given */
  int annotate_given;	 /* Whether annotate was given */
  int no_annotate_given;	 /* Whether no-annotate was given */
  int abbrevs_given;	 /* Whether abbrevs was given */
  int conjunctions_given;	 /* Whether conjunctions was given */
  int stopwords_given;	 /* Whether stopwords was given */
  int dehyphenate_given;	 /* Whether dehyphenate was given */
  int no_dehyphenate_given;	 /* Whether no-dehyphenate was given */
  int model_given;	 /* Whether model was given */
  int input_format_given;	 /* Whether input-format was given */
  int output_format_given;	 /* Whether output-format was given */
  
  char **inputs;         /* unnamed arguments */
  unsigned inputs_num;   /* number of unnamed arguments */
};

/* read rc files (if any) and parse all command-line options in one swell foop */
int  cmdline_parser (int argc, char *const *argv, struct gengetopt_args_info *args_info);

/* instantiate defaults from environment variables: you must call this yourself! */
void cmdline_parser_envdefaults (struct gengetopt_args_info *args_info);

/* read a single rc-file */
void cmdline_parser_read_rcfile (const char *filename,
				    struct gengetopt_args_info *args_info,
				    int user_specified);

/* read a single rc-file (stream) */
void cmdline_parser_read_rc_stream (FILE *rcfile,
				       const char *filename,
				       struct gengetopt_args_info *args_info);

/* parse a single option */
int cmdline_parser_parse_option (char oshort, const char *olong, const char *val,
				    struct gengetopt_args_info *args_info);

/* print help message */
void cmdline_parser_print_help(void);

/* print version */
void cmdline_parser_print_version(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* waste_cmdparser_h */
