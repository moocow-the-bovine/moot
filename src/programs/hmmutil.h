/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: hmmutil.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + HMM command-line parsing utilities : headers
 *--------------------------------------------------------------------------*/

#ifndef _HMMUTIL_H
#define _HMMUTIL_H

#include <stdio.h>

/*--------------------------------------------------------------------
 * utility functions: parsing
 *---------------------------------------------------------------------*/

/**
 * Parse string 'model' into 'binfile', 'lexfile', and 'ngfile'
 * parts.  Rules:
 *  1) If file (model) exists, then binfile="MODEL",
 *     'lexfile' and 'ngfile' are untouched.
 *  2) Else, if (model) contains a comma (model ~= "MODEL.lex,MODEL.123"),
 *     lexfile="MODEL.lex", ngfile="MODEL.123", binfile=(untouched)
 *  3) Else, lexfile="(model).lex", ngfile="(model).123", binfile=(untouched).
 * 
 * The assigned (*file) strings will be allocated if they are NULL.
 */
bool hmm_parse_model(char *model, char **binfile, char **lexfile, char **ngfile);

/** As above, text only */
bool hmm_parse_textmodel(char *model, char **lexfile, char **ngfile);

/** Similar, uses corpus basename  */
bool hmm_parse_corpusmodel(char *corpus, char **lexfile, char **ngfile);

/**
 * Parse a comma-separated list of doubles (at most 'ndbls') from str into dbls.
 * You should already have allocated 'dbls'.
 */
bool hmm_parse_doubles(char *str, double *dbls, size_t ndbls);


#endif // _CMDUTIL_H
