/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2002-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: hmmutil.cc
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


bool hmm_parse_textmodel(char *model, char **lexfile, char **ngfile, char **lcfile)
{
  char *comma = strchr(model, ',');
  char *comma2 = comma ? strchr(comma+1,',') : NULL;
  if (comma) {
    *comma = '\0';
    if (!*lexfile) *lexfile = (char *)malloc(strlen(model)+1);
    strcpy(*lexfile, model);
    *comma = ',';
    
    comma++;
    if (comma2) *comma2 = '\0';
    if (!*ngfile) *ngfile = (char *)malloc(strlen(comma)+1);
    strcpy(*ngfile, comma);

    if (comma2) {
      *comma2 = ',';
      if (lcfile) {
	comma2++;
	if (!*lcfile) *lcfile = (char *)malloc(strlen(comma2)+1);
	strcpy(*lcfile, comma2);
      }
    }

    return true;
  }
  
  if (!*lexfile) *lexfile = (char *)malloc(strlen(model)+4);
  strcpy(*lexfile, model);
  strcat(*lexfile, ".lex");

  if (!*ngfile)  *ngfile = (char *)malloc(strlen(model)+4);
  strcpy(*ngfile, model);
  strcat(*ngfile, ".123");

  if (lcfile) {
    if (!*lcfile)  *lcfile = (char *)malloc(strlen(model)+4);
    strcpy(*lcfile, model);
    strcat(*lcfile, ".clx");
  }

  return true;
}

bool hmm_parse_corpusmodel(char *corpus, char **lexfile, char **ngfile, char **lcfile)
{
  char olddot = '.';
  char *dot   = strrchr(corpus, '.');
  if (!dot) {
    dot = corpus + strlen(corpus);
    olddot = '\0';
  }
  *dot = '\0';

  if (!*lexfile) *lexfile = (char *)malloc(strlen(corpus)+4);
  strcpy(*lexfile, corpus);
  strcat(*lexfile, ".lex");

  if (!*ngfile)  *ngfile = (char *)malloc(strlen(corpus)+4);
  strcpy(*ngfile, corpus);
  strcat(*ngfile, ".123");

  if (lcfile) {
    if (!*lcfile)  *lcfile = (char *)malloc(strlen(corpus)+4);
    strcpy(*lcfile, corpus);
    strcat(*lcfile, ".clx");
  }

  *dot = olddot;
  return true;
}

bool hmm_parse_model(char *model, char **binfile, char **lexfile, char **ngfile,
		     char **lcfile)
{
  //-- binary file: easy
  if (file_exists(model)) {
    if (!*binfile) *binfile = (char *)malloc(strlen(model)+1);
    strcpy(*binfile, model);
    return true;
  }
  return hmm_parse_textmodel(model, lexfile, ngfile, lcfile);
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
