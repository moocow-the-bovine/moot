/* -*- Mode: C -*-
 *
 * File: mootdyn_cmdparser.h
 * Description: Headers for command-line parser struct gengetopt_args_info.
 *
 * File autogenerated by optgen.perl version 0.07.
 *
 */

#ifndef mootdyn_cmdparser_h
#define mootdyn_cmdparser_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * moocow: Never set PACKAGE and VERSION here.
 */

struct gengetopt_args_info {
  char * rcfile_arg;	 /* Read an alternate configuration file. (default=NULL). */
  int verbose_arg;	 /* Verbosity level. (default=4). */
  int no_banner_flag;	 /* Suppress initial banner message (implied at verbosity levels <= 2) (default=0). */
  int no_header_flag;	 /* Suppres leading comments in destination file. (default=0). */
  int dots_arg;	 /* Print a dot for every NTOKS tokens processed. (default=0). */
  int list_flag;	 /* INPUTs are file-lists, not filenames. (default=0). */
  int recover_flag;	 /* Attempt to recover from minor errors. (default=0). */
  char * output_arg;	 /* Specify output file (default=stdout). (default=-). */
  char * input_format_arg;	 /* Specify input file(s) format(s). (default=NULL). */
  char * output_format_arg;	 /* Specify output file format. (default=NULL). */
  char * input_encoding_arg;	 /* Override XML document input encoding. (default=NULL). */
  char * output_encoding_arg;	 /* Set default XML output encoding. (default=NULL). */
  char * model_arg;	 /* Use HMM model file(s) MODEL. (default=moothmm). */
  int hash_ngrams_arg;	 /* Whether to hash stored n-grams (default=yes) (default=1). */
  int trie_depth_arg;	 /* Maximum depth of suffix trie. (default=0). */
  int trie_threshhold_arg;	 /* Frequency upper bound for trie inclusion. (default=10). */
  float trie_theta_arg;	 /* Suffix backoff coefficient. (default=0). */
  int use_classes_arg;	 /* Whether to use lexical class-probabilities. (default=1). */
  int relax_arg;	 /* Whether to relax token-tag associability (default=1 (true)) (default=1). */
  char * nlambdas_arg;	 /* N-Gram smoothing constants (default=estimate) (default=NULL). */
  char * wlambdas_arg;	 /* Lexical smoothing constants (default=estimate) (default=NULL). */
  char * clambdas_arg;	 /* Lexical-class smoothing constants (default=estimate) (default=NULL). */
  double unknown_threshhold_arg;	 /* Freq. threshhold for 'unknown' lexical probabilities (default=1.0). */
  double class_threshhold_arg;	 /* Freq. threshhold for 'unknown' class probabilities (default=1.0). */
  char * unknown_token_arg;	 /* Symbolic name of the 'unknown' token (default=@UNKNOWN). */
  char * unknown_tag_arg;	 /* Symbolic name of the 'unknown' tag (default=UNKNOWN). */
  char * eos_tag_arg;	 /* Specify boundary tag (default=__$) (default=__$). */
  double beam_width_arg;	 /* Specify cutoff factor for beam pruning (default=1000). */
  int save_ambiguities_flag;	 /* Annotate tagged tokens with lexical ambiguities (default=0). */
  int mark_unknown_flag;	 /* Mark unknown tokens. (default=0). */
  char * dyn_class_arg;	 /* Specify built-in dynamic estimator (default='Freq') (default=Freq). */
  int dyn_invert_arg;	 /* Estimate p(w|t)~=p(t|w)? (default=1) (default=1). */
  float dyn_base_arg;	 /* Base for Maxwell-Boltzmann estimator (default=2) (default=2.0). */
  float dyn_beta_arg;	 /* Temperature coefficient for Maxwell-Boltzmann estimator (default=1) (default=1.0). */
  char * dyn_new_tag_arg;	 /* Specify pseudo-tag for new analyses (default='@NEW') (default=@NEW). */
  float dyn_freq_eps_arg;	 /* Specify dynamic lexical pseudo-frequency smoothing constant (default=0.1) (default=0.1). */
  int dyn_text_tags_flag;	 /* Use token text field as n-gram source for MIParser (default=0). */

  int help_given;	 /* Whether help was given */
  int version_given;	 /* Whether version was given */
  int rcfile_given;	 /* Whether rcfile was given */
  int verbose_given;	 /* Whether verbose was given */
  int no_banner_given;	 /* Whether no-banner was given */
  int no_header_given;	 /* Whether no-header was given */
  int dots_given;	 /* Whether dots was given */
  int list_given;	 /* Whether list was given */
  int recover_given;	 /* Whether recover was given */
  int output_given;	 /* Whether output was given */
  int input_format_given;	 /* Whether input-format was given */
  int output_format_given;	 /* Whether output-format was given */
  int input_encoding_given;	 /* Whether input-encoding was given */
  int output_encoding_given;	 /* Whether output-encoding was given */
  int model_given;	 /* Whether model was given */
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
  int save_ambiguities_given;	 /* Whether save-ambiguities was given */
  int mark_unknown_given;	 /* Whether mark-unknown was given */
  int dyn_class_given;	 /* Whether dyn-class was given */
  int dyn_invert_given;	 /* Whether dyn-invert was given */
  int dyn_base_given;	 /* Whether dyn-base was given */
  int dyn_beta_given;	 /* Whether dyn-beta was given */
  int dyn_new_tag_given;	 /* Whether dyn-new-tag was given */
  int dyn_freq_eps_given;	 /* Whether dyn-freq-eps was given */
  int dyn_text_tags_given;	 /* Whether dyn-text-tags was given */
  
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
#endif /* mootdyn_cmdparser_h */
