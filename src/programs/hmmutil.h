/* -*- Mode: C++ -*- */
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
bool hmm_parse_model(char *model,
		     char **binfile,
		     char **lexfile,
		     char **ngfile,
		     char **lcfile=NULL);

/** As above, text only */
bool hmm_parse_textmodel(char *model,
			 char **lexfile,
			 char **ngfile,
			 char **lcfile=NULL);

/** Similar, uses corpus basename  */
bool hmm_parse_corpusmodel(char *corpus,
			   char **lexfile,
			   char **ngfile,
			   char **lcfile=NULL);

/**
 * Parse a comma-separated list of doubles (at most 'ndbls') from str into dbls.
 * You should already have allocated 'dbls'.
 */
bool hmm_parse_doubles(char *str, double *dbls, size_t ndbls);


#endif // _CMDUTIL_H
