/*--------------------------------------------------------------------------
 * File: hmmdutil.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + HMM command-line parsing utilities
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cmdutil.h"

#ifdef HAVE_CONFIG_H
# include <dwdstConfig.h>
#endif

/*--------------------------------------------------------------------
 * utility functions: parsing
 *---------------------------------------------------------------------*/


bool hmm_parse_textmodel(char *model, char **lexfile, char **ngfile)
{
  char *comma = strchr(model, ',');
  if (comma) {
    *comma = '\0';
    if (!*lexfile) *lexfile = (char *)malloc(strlen(model)+1);
    strcpy(*lexfile, model);
    *comma = ',';
    
    comma++;
    if (!*ngfile) *ngfile = (char *)malloc(strlen(comma)+1);
    strcpy(*ngfile, comma);

    return true;
  }
  
  if (!*lexfile) *lexfile = (char *)malloc(strlen(model)+4);
  if (!*ngfile)  *ngfile = (char *)malloc(strlen(model)+4);
  strcpy(*lexfile, model);
  strcpy(*ngfile, model);
  strcat(*lexfile, ".lex");
  strcat(*ngfile, ".123");
  return true;
}

bool hmm_parse_corpusmodel(char *corpus, char **lexfile, char **ngfile)
{
  char *dot   = strrchr(corpus, '.');
  *dot = '\0';
  if (!*lexfile) *lexfile = (char *)malloc(strlen(corpus)+4);
  if (!*ngfile)  *ngfile = (char *)malloc(strlen(corpus)+4);
  strcpy(*lexfile, corpus);
  strcpy(*ngfile, corpus);
  strcat(*lexfile, ".lex");
  strcat(*ngfile, ".123");
  *dot = '.';
  return true;
}

bool hmm_parse_model(char *model, char **binfile, char **lexfile, char **ngfile)
{
  //-- binary file: easy
  if (file_exists(model)) {
    if (!*binfile) *binfile = (char *)malloc(strlen(model)+1);
    strcpy(*binfile, model);
    return true;
  }
  return hmm_parse_textmodel(model, lexfile, ngfile);
}



bool hmm_parse_doubles(char *str, double *dbls, size_t ndbls)
{
  size_t i;
  char  *s, *comma;
  if (!str) return false;
  for (i = 0, s = str; i < ndbls; i++, s = comma+1) {
    dbls[i] = strtod(s, &comma);
    comma = strchr(comma, ',');
    if (!comma) break;
  }
  return true;
}
