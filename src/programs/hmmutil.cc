/*
   moot-utils version 1.0.4 : moocow's part-of-speech tagger
   Copyright (C) 2002-2003 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

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
# include <config.h>
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
