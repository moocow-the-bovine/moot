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
 * Parse string 'model' into lexical-file and ngram-file parts.
 * (*lexfile) and (*ngfile) will be allocated if they are NULL.
 */
bool hmm_parse_model(char *model, char **lexfile, char **ngfile);

/**
 * Parse a comma-separated list of doubles (at most 'ndbls') from str into dbls.
 * You should already have allocated 'dbls'.
 */
bool hmm_parse_doubles(char *str, double *dbls, size_t ndbls);


#endif // _CMDUTIL_H
