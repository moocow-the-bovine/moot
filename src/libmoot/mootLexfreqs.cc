/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2012 by Bryan Jurish <moocow@cpan.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*============================================================================
 * File: mootLexfreqs.cc
 * Author:  Bryan Jurish <moocow@cpan.org>
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

void mootLexfreqs::add_count(const mootTokString &text,
			     const mootTagString &tag,
			     const LexfreqCount count)
{
  //-- adjust token-table
  LexfreqTokTable::iterator lfi = lftable.find(text);
  if (lfi == lftable.end()) {
    //-- new token
    lfi = lftable.insert(LexfreqTokTable::value_type(text,LexfreqEntry(count))).first;
    lfi->second.freqs[tag] = count;
  } else {
    //-- known token
    lfi->second.count += count;

    LexfreqSubtable::iterator lsi = lfi->second.freqs.find(tag);
    if (lsi == lfi->second.freqs.end()) {
      //-- unknown (tok,tag) pair
      lfi->second.freqs[tag] = count;
    } else {
      //-- known (tok,tag) pair: just add
      lsi->second += count;
    }
  }

  if (!isTokFlavorName(text)) {
    //-- adjust total tag-count
    LexfreqTagTable::iterator lftagi = tagtable.find(tag);
    if (lftagi != tagtable.end()) {
      lftagi->second += count;
    } else {
      tagtable[tag] = count;
    }

    //-- adjust total token-count
    n_tokens += count;
  }
};


/*----------------------------------------------------------------------
 * Lookup
 *----------------------------------------------------------------------*/

// (inlined)

/*----------------------------------------------------------------------
 * Compilation
 *----------------------------------------------------------------------*/
void mootLexfreqs::compute_specials(void)
{
  string unknown_str("@UNKNOWN");

  //-- ensure entries for all known flavors exist
  set<mootFlavorStr> flavors = taster.labels();
  if (flavors.find(taster.nolabel)!=flavors.end()) {
    flavors.erase(flavors.find(taster.nolabel));
  }
  if (unknown_threshhold > 0) {
    flavors.insert(unknown_str);
  }
  for (set<mootFlavorStr>::const_iterator fi=flavors.begin(); fi!=flavors.end(); ++fi) {
    LexfreqTokTable::iterator lfi = lftable.find(*fi);
    if (lfi == lftable.end())
      lftable.insert(LexfreqTokTable::value_type(*fi, LexfreqEntry()));
  }

  //-- iterate over all tokens
  for (LexfreqTokTable::const_iterator lfti = lftable.begin(); lfti != lftable.end(); ++lfti) {
    const mootFlavorStr& flav = taster.flavor(lfti->first);

    if (flavors.find(lfti->first) != flavors.end()) {
      //-- don't merge pseudo-types into other pseudo-types
      continue;
    }
    else if (flav != taster.nolabel) {
      //-- found a special: add its counts to proper subtable
      for (LexfreqSubtable::const_iterator lsi = lfti->second.freqs.begin(); lsi != lfti->second.freqs.end(); ++lsi) {
	add_count(flav, lsi->first, lsi->second);
      }
    }
    else if (lfti->second.count <= unknown_threshhold && unknown_threshhold > 0) {
      //-- found a pseudo-unknown token: add its counts to the "@UNKNOWN" entry
      for (LexfreqSubtable::const_iterator lsi = lfti->second.freqs.begin(); lsi != lfti->second.freqs.end(); ++lsi) {
	add_count(unknown_str, lsi->first, lsi->second);
      }
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
      if (isTokFlavorName(lfti->first)) continue; //-- ignore specials
      n += lfti->second.freqs.size();
    }
  return n;
};


/*----------------------------------------------------------------------
 * I/O
 *----------------------------------------------------------------------*/
bool mootLexfreqs::load(const char *filename)
{
  FILE *file = (strcmp(filename,"-")==0 ? stdin : fopen(filename,"r"));
  if (!file) {
    fprintf(stderr, "mootLexfreqs::load(): open failed for file '%s': %s\n",
	    filename,
	    strerror(errno));
    return 0;
  }
  bool rc = load(file,filename);
  if (file != stdin) fclose(file);
  return rc;
}

bool mootLexfreqs::load(FILE *file, const char *filename)
{
  mootLexfreqsCompiler lfcomp;
  lfcomp.srcname = filename;
  lfcomp.lexfreqs  = this;
  bool rc = (lfcomp.parse_from_file(file) != NULL);
  lfcomp.srcname = NULL;
  return rc;
}

bool mootLexfreqs::save(const char *filename)
{
  FILE *file = (strcmp(filename,"-")==0 ? stdout : fopen(filename,"w"));
  if (!file) {
    fprintf(stderr, "mootLexfreqs::save(): open failed for file '%s': %s\n",
	    filename,
	    strerror(errno));
    return 0;
  }
  bool rc = save(file);
  if (file != stdout) fclose(file);
  return rc;
}

bool mootLexfreqs::save(FILE *file, const char *filename)
{
  set<mootTokString> toks;

#if 1
  //-- save flavors
  if (taster.rules.size()>0 || !taster.nolabel.empty()) {
    fprintf(file,"%%%%\n");
    for (mootTaster::Rules::const_iterator ri=taster.rules.begin(); ri!=taster.rules.end(); ++ri) {
      fprintf(file, "%%%%$FLAVOR %s\t%s\n", ri->lab.c_str(), ri->re_s.c_str());
    }
    if (!taster.nolabel.empty()) {
      fprintf(file, "%%%%$FLAVOR DEFAULT %s\n", taster.nolabel.c_str());
    }
  }
#endif

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

