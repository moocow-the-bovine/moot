/* -*- Mode: C++ -*- */

/*
   libmoot version 1.0.4 : moocow's part-of-speech tagging library
   Copyright (C) 2003 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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

/*============================================================================
 * File: mootNgrams.cc
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *    Class for storage & retrieval of N-Gram counts
 *============================================================================*/

#include <stdio.h>
#include <errno.h>

#include <mootNgrams.h>
#include <mootNgramsCompiler.h>

moot_BEGIN_NAMESPACE

/*----------------------------------------------------------------------
 * Manipulators
 *----------------------------------------------------------------------*/

// (inlined)

/*----------------------------------------------------------------------
 * Lookup
 *----------------------------------------------------------------------*/

// (inlined)


/*----------------------------------------------------------------------
 * I/O
 *----------------------------------------------------------------------*/
bool mootNgrams::load(char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "mootNgrams::load(): open failed for file '%s': %s\n",
	    filename,
	    strerror(errno));
    return 0;
  }
  bool rc = load(file, filename);
  fclose(file);
  return rc;
}

bool mootNgrams::load(FILE *file, char *filename)
{
  mootNgramsCompiler ngcomp;
  ngcomp.srcname = filename;
  ngcomp.ngrams  = this;
  return (ngcomp.parse_from_file(file) != NULL);
}

bool mootNgrams::save(char *filename, bool compact)
{
  FILE *file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "mootNgrams::save(): open failed for file '%s': %s\n",
	    filename,
	    strerror(errno));
    return 0;
  }
  bool rc = save(file, filename, compact);
  fclose(file);
  return rc;
}

bool mootNgrams::save(FILE *file, char *filename, bool compact)
{
  set<NgramString> ngrams;

  //-- prepare sorted key-list
  for (NgramStringTable::const_iterator ngti = ngtable.begin(); ngti != ngtable.end(); ngti++) {
    ngrams.insert(ngti->first);
  }

  //-- and output
  NgramString::const_iterator ngii;
  NgramString                 prevngram;
  NgramString::const_iterator pngi;
  for (set<NgramString>::iterator ngi = ngrams.begin(); ngi != ngrams.end(); ngi++) {
    if (ngi->empty()) continue;
    if (compact) {
      for (pngi  = prevngram.begin() ,  ngii  = ngi->begin();
	   pngi != prevngram.end()   && ngii != ngi->end()    && *pngi == *ngii;
	   pngi ++                   ,  ngii ++)
	{
	  fputc('\t', file);
	}
      for (; ngii != ngi->end(); ngii++)
	{
	  fputs(ngii->c_str(), file);
	  fputc('\t', file);
	}
      prevngram = *ngi;
    }
    else {
      //-- verbose mode: print all tags
      for (ngii = ngi->begin(); ngii != ngi->end(); ngii++) {
	fputs(ngii->c_str(), file);
	fputc('\t', file);
      }
    }
    fprintf(file, "%g\n", ngtable[*ngi]);
  }

  return 1;
}

moot_END_NAMESPACE
