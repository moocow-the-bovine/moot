/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
#include <string.h>

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
  tagtable.clear();
}

/*----------------------------------------------------------------------
 * Lookup
 *----------------------------------------------------------------------*/

// (inlined)

/*----------------------------------------------------------------------
 * Compilation
 *----------------------------------------------------------------------*/
void mootLexfreqs::compute_specials(void)
{
  LexfreqEntry *entries[NTokTypes];
  int typ;
  
  //-- initialize entry pointers
  for (typ = 0; typ < NTokTypes; typ++) {
    if (typ == TokTypeAlpha) { //  || typ == TokTypeUnknown
      entries[typ] = NULL;
    } else {
      entries[typ] = &(lftable[TokenTypeNames[typ]]);
      entries[typ]->clear();
    }
  }

  //-- iterate over all tokens
  for (LexfreqTokTable::const_iterator lfti = lftable.begin();
       lfti != lftable.end();
       lfti++)
    {
      typ = token2type(lfti->first);
      if (lfti->first[0] == '@' || typ == TokTypeAlpha) // || typ == TokTypeUnknown
	continue;
      
      //-- found a special: add its counts to proper subtable
      entries[typ]->count += lfti->second.count;
      for (LexfreqSubtable::const_iterator lsi = lfti->second.freqs.begin();
	   lsi != lfti->second.freqs.end();
	   lsi++)
	{
	  entries[typ]->freqs[lsi->first] += lsi->second;
	}
    }
};

/*----------------------------------------------------------------------
 * Information
 *----------------------------------------------------------------------*/
size_t mootLexfreqs::n_pairs(void)
{
  size_t n = 0;

  //-- iterate over all tokens
  for (LexfreqTokTable::const_iterator lfti = lftable.begin();
       lfti != lftable.end();
       lfti++)
    {
      if (lfti->first[0] == '@') continue; //-- ignore specials
      n += lfti->second.freqs.size();
    }
  return n;
};


/*----------------------------------------------------------------------
 * I/O
 *----------------------------------------------------------------------*/
bool mootLexfreqs::load(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "mootLexfreqs::load(): open failed for file '%s': %s\n",
	    filename,
	    strerror(errno));
    return 0;
  }
  bool rc = load(file,filename);
  fclose(file);
  return rc;
}

bool mootLexfreqs::load(FILE *file, const char *filename)
{
  mootLexfreqsCompiler lfcomp;
  lfcomp.srcname = strdup(filename);
  lfcomp.lexfreqs  = this;
  bool rc = (lfcomp.parse_from_file(file) != NULL);
  free(lfcomp.srcname);
  lfcomp.srcname = NULL;
  return rc;
}

bool mootLexfreqs::save(const char *filename)
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

bool mootLexfreqs::save(FILE *file, const char *filename)
{
  set<mootTokString> toks;

  //-- prepare sorted key-list
  for (LexfreqTokTable::const_iterator lfi = lftable.begin(); lfi != lftable.end(); lfi++) {
    toks.insert(lfi->first);
  }

  //-- iterate through sorted keys
  for (set<mootTokString>::const_iterator toki = toks.begin(); toki != toks.end(); toki++) {
    const LexfreqEntry  &entry  = lftable[*toki];
    fprintf(file, "%s\t%g", toki->c_str(), entry.count);

    for (LexfreqSubtable::const_iterator ei = entry.freqs.begin(); ei != entry.freqs.end(); ei++) {
      fprintf(file, "\t%s\t%g", ei->first.c_str(), ei->second);
    }
    fputc('\n', file);
  }
  return 1;
}

moot_END_NAMESPACE

