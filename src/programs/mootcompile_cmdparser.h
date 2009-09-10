/* -*- Mode: C -*-
 *
 * File: mootcompile_cmdparser.h
 * Description: Headers for command-line parser struct gengetopt_args_info.
 *
 * File autogenerated by optgen.perl version 0.06.
 *
 */

#ifndef mootcompile_cmdparser_h
#define mootcompile_cmdparser_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * moocow: Never set PACKAGE and VERSION here.
 */

struct gengetopt_args_info {
  char * rcfile_arg;	 /* Read an alternate configuration file. (default=NULL). */
  int verbose_arg;	 /* Verbosity level. (default=2). */
  char * output_arg;	 /* Specify output file (default=stdout). (default=-). */
  int compress_arg;	 /* Compression level for output file. (default=-1). */
  int hash_ngrams_arg;	 /* Whether to hash stored n-grams (default=no) (default=0). */
  int trie_depth_arg;	 /* Maximum depth of suffix trie. (default=0). */
  int trie_threshhold_arg;	 /* Frequency upper bound for trie inclusion. (default=10). */
  float trie_theta_arg;	 /* Suffix backoff coefficient. (default=0). */
  int use_classes_arg;	 /* Whether to use lexical class-probabilities. (default=1). */
  int relax_arg;	 /* Whether to relax token-tag associability (default=no) (default=0). */
  char * nlambdas_arg;	 /* N-Gram smoothing constants (default=estimate) (default=NULL). */
  char * wlambdas_arg;	 /* Lexical smoothing constants (default=estimate) (default=NULL). */
  char * clambdas_arg;	 /* Lexical-class smoothing constants (default=estimate) (default=NULL). */
  double unknown_threshhold_arg;	 /* Freq. threshhold for 'unknown' lexical probabilities (default=1.0). */
  double class_threshhold_arg;	 /* Freq. threshhold for 'unknown' class probabilities (default=1.0). */
  char * unknown_token_arg;	 /* Symbolic name of the 'unknown' token (default=@UNKNOWN). */
  char * unknown_tag_arg;	 /* Symbolic name of the 'unknown' tag (default=UNKNOWN). */
  char * eos_tag_arg;	 /* Specify boundary tag (default=__$) (default=__$). */
  double beam_width_arg;	 /* Specify cutoff factor for beam pruning (default=1000). */

  int help_given;	 /* Whether help was given */
  int version_given;	 /* Whether version was given */
  int rcfile_given;	 /* Whether rcfile was given */
  int verbose_given;	 /* Whether verbose was given */
  int output_given;	 /* Whether output was given */
  int compress_given;	 /* Whether compress was given */
  int hash_ngrams_given;	 /* Whether hash-ngrams was given */
  int trie_depth_given;	 /* Whether trie-depth was given */
  int trie_threshhold_given;	 /* Whether trie-threshhold was given */
  int trie_theta_given;	 /* Whether trie-theta was given */
  int use_classes_given;	 /* Whether use-classes was given */
  int relax_given;	 /* Whether relax was given */
  int nlambdas_given;	 /* Whether nlambdas was given */
  int wlambdas_given;	 /* Whether wlambdas was given */
  int clambdas_given;	 /* Whether clambdas was given */
  int unknown_threshhold_given;	 /* Whether unknown-threshhold was given */
  int class_threshhold_given;	 /* Whether class-threshhold was given */
  int unknown_token_given;	 /* Whether unknown-token was given */
  int unknown_tag_given;	 /* Whether unknown-tag was given */
  int eos_tag_given;	 /* Whether eos-tag was given */
  int beam_width_given;	 /* Whether beam-width was given */
  
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
#endif /* mootcompile_cmdparser_h */
