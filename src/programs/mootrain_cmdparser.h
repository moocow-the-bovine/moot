/* -*- Mode: C -*-
 *
 * File: mootrain_cmdparser.h
 * Description: Headers for command-line parser struct gengetopt_args_info.
 *
 * File autogenerated by optgen.perl version 0.04.
 *
 */

#ifndef mootrain_cmdparser_h
#define mootrain_cmdparser_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * moocow: Never set PACKAGE and VERSION here.
 */

struct gengetopt_args_info {
  char * rcfile_arg;	 /* Read an alternate configuration file. (default=NULL). */
  int verbose_arg;	 /* Verbosity level. (default=2). */
  char * output_arg;	 /* Specify basename for output files (default=INPUT) (default=NULL). */
  char * input_format_arg;	 /* Specify input file(s) format(s). (default=NULL). */
  char * input_encoding_arg;	 /* Override document encoding for XML input. (default=NULL). */
  int lex_flag;	 /* Generate only lexical frequency file. (default=0). */
  int ngrams_flag;	 /* Generate only n-gram frequency file. (default=0). */
  int classes_flag;	 /* Generate only lexical-class frequency file. (default=0). */
  char * eos_tag_arg;	 /* Specify boundary tag (default=__$) (default=__$). */
  int verbose_ngrams_flag;	 /* Generate long-form ngrams (default=no) (default=0). */

  int help_given;	 /* Whether help was given */
  int version_given;	 /* Whether version was given */
  int rcfile_given;	 /* Whether rcfile was given */
  int verbose_given;	 /* Whether verbose was given */
  int output_given;	 /* Whether output was given */
  int input_format_given;	 /* Whether input-format was given */
  int input_encoding_given;	 /* Whether input-encoding was given */
  int lex_given;	 /* Whether lex was given */
  int ngrams_given;	 /* Whether ngrams was given */
  int classes_given;	 /* Whether classes was given */
  int eos_tag_given;	 /* Whether eos-tag was given */
  int verbose_ngrams_given;	 /* Whether verbose-ngrams was given */
  
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
#endif /* mootrain_cmdparser_h */
