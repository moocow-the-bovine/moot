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
 * File: mootLexfreqs.cc
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *    Class for storage & retrieval of lexical frequency parameters
 *============================================================================*/

#include <stdio.h>
#include <errno.h>

#include <mootLexfreqs.h>
#include <mootLexfreqsCompiler.h>

moot_BEGIN_NAMESPACE

/*----------------------------------------------------------------------
 * Manipulators
 *----------------------------------------------------------------------*/

void mootLexfreqs::clear(void)
{
  n_tokens = 0;
  lftable.clear();
  lftotals.clear();
  lftags.clear();
}

/*----------------------------------------------------------------------
 * Lookup
 *----------------------------------------------------------------------*/

// (inlined)


/*----------------------------------------------------------------------
 * I/O
 *----------------------------------------------------------------------*/
bool mootLexfreqs::load(char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "mootLexfreqs::load(): open failed for file '%s': %s\n",
	    filename,
	    strerror(errno));
    return 0;
  }
  bool rc = load(file);
  fclose(file);
  return rc;
}

bool mootLexfreqs::load(FILE *file, char *filename)
{
  mootLexfreqsCompiler lfcomp;
  lfcomp.srcname = filename;
  lfcomp.lexfreqs  = this;
  return (lfcomp.parse_from_file(file) != NULL);
}

bool mootLexfreqs::save(char *filename)
{
  FILE *file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "mootLexfreqs::save(): open failed for file '%s': %s\n",
	    filename,
	    strerror(errno));
    return 0;
  }
  bool rc = save(file);
  fclose(file);
  return rc;
}

bool mootLexfreqs::save(FILE *file, char *filename)
{
  set<LexfreqKey> keys;

  //-- prepare sorted key-list
  for (LexfreqStringTable::const_iterator lfti = lftable.begin(); lfti != lftable.end(); lfti++) {
    keys.insert(lfti->first);
  }

  //-- iterate through sorted keys
  mootTokString lasttok;
  for (set<LexfreqKey>::const_iterator keyi = keys.begin(); keyi != keys.end(); keyi++) {
    const mootTokString &curtok = keyi->first;
    const mootTagString &curtag = keyi->second;

    //-- do we have a new token?
    if (keyi == keys.begin() || curtok != lasttok) {
      if (keyi != keys.begin()) fputc('\n', file);
      fprintf(file, "%s\t%g", curtok.c_str(), lookup(curtok));
      lasttok = curtok;
    }

    //-- output tags
    fprintf(file, "\t%s\t%g", curtag.c_str(), lookup(*keyi));
  }
  fputc('\n', file);

  return 1;
}

moot_END_NAMESPACE

