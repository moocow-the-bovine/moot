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

//--------------------------------------------------------------
void mootLexfreqs::clear(void)
{
  n_tokens = 0;
  lftable.clear();
  tagtable.clear();
}

//--------------------------------------------------------------
void mootLexfreqs::add_count(const mootTokString &text, const mootTagString &tag, const LexfreqCount count)
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

  //-- adjust total tag-count (pre-v2.0.9-1, we didn't add add tag- or total-token-counts for pseudo-lexemes here)
  LexfreqTagTable::iterator lftagi = tagtable.find(tag);
  if (lftagi != tagtable.end()) {
    lftagi->second += count;
  } else {
    tagtable[tag] = count;
  }

  //-- adjust total token-count
  n_tokens += count;
};

//--------------------------------------------------------------
void mootLexfreqs::remove_word(const mootTokString &text)
{
  //-- adjust (text->tag->freq) table
  LexfreqTokTable::iterator lfi = lftable.find(text);
  if (lfi==lftable.end()) return;

  //-- adjust tag counts
  for (LexfreqSubtable::const_iterator lsi=lfi->second.freqs.begin(); lsi != lfi->second.freqs.end(); ++lsi) {
    tagtable[lsi->first] -= lsi->second;
    n_tokens             -= lsi->second;
  }

  //-- remove entry
  lftable.erase(lfi);
}

/*----------------------------------------------------------------------
 * Lookup
 *----------------------------------------------------------------------*/

// (inlined)

/*----------------------------------------------------------------------
 * Compilation
 *----------------------------------------------------------------------*/
void mootLexfreqs::compute_specials(bool compute_unknown)
{
  string unknown_str("@UNKNOWN");
  mootFlavorStr noFlavor(""); //-- dummy string used when taster==NULL

  //-- ensure entries for all known flavors exist
  set<mootFlavorStr> flavors;
  if (taster != NULL) {
    flavors = taster->labels;
    if (flavors.find(taster->nolabel)!=flavors.end())
      flavors.erase(flavors.find(taster->nolabel));   //-- don't track counts for the default flavor
  }

  if (compute_unknown && unknown_threshhold > 0)
    flavors.insert(unknown_str);

  //-- ensure flavor entries exist, but don't overwrite any existing entries
  set<mootFlavorStr> all_flavors = flavors;
  all_flavors.insert(unknown_str);
  for (set<mootFlavorStr>::const_iterator fi=all_flavors.begin(); fi!=all_flavors.end(); ++fi) {
    if (f_text(*fi) != 0) {
      //-- existing entry: remove from flavors
      flavors.erase(*fi);
      //moot_carp("compute_specials(): NOT re-computing counts for existing pseudo-lexeme '%s'\n", fi->c_str()); //--DEBUG
    } else {
      //-- no entry exists (or f==0): create one
      lftable.insert(LexfreqTokTable::value_type(*fi, LexfreqEntry()));
    }
  }
  if (flavors.find("@UNKNOWN")==flavors.end())
      compute_unknown = false;

  //-- iterate over all tokens
  for (LexfreqTokTable::const_iterator lfti = lftable.begin(); lfti != lftable.end(); ++lfti) {
    const mootFlavorStr& flav = taster ? taster->flavor(lfti->first) : noFlavor;

    if (all_flavors.find(lfti->first) != all_flavors.end()) {
      //-- pseudo-lexeme: don't merge it into any (other) pseudo-lexeme, even ones we're not computing
      continue;
    }
    else if (taster && flav != taster->nolabel && flavors.find(lfti->first) == flavors.end()) {
      //-- found a flavor-special: add its counts to the appropriate flavor entry
      for (LexfreqSubtable::const_iterator lsi = lfti->second.freqs.begin(); lsi != lfti->second.freqs.end(); ++lsi) {
	add_count(flav, lsi->first, lsi->second);
      }
    }
    else if (compute_unknown && unknown_threshhold > 0 && lfti->second.count <= unknown_threshhold) {
      //-- found a pseudo-unknown: add its counts to the "@UNKNOWN" entry
      for (LexfreqSubtable::const_iterator lsi = lfti->second.freqs.begin(); lsi != lfti->second.freqs.end(); ++lsi) {
	add_count(unknown_str, lsi->first, lsi->second);
      }
    }
  }
};

//----------------------------------------------------------------------
void mootLexfreqs::remove_specials(bool remove_unknown)
{
  set<mootFlavorStr> flavors;
  if (taster)
    flavors = taster->labels;
  if (remove_unknown)
    flavors.insert("@UNKNOWN");
  for (set<mootFlavorStr>::const_iterator fi=flavors.begin(); fi!=flavors.end(); ++fi) {
    remove_word(*fi);
  }
}

//----------------------------------------------------------------------
void mootLexfreqs::discount_specials(CountT zf_special)
{
  set<mootFlavorStr> flavors;
  set<mootFlavorStr>::const_iterator fi;
  if (taster) flavors = taster->labels;
  flavors.insert("@UNKNOWN");

  //-- get unadjusted total count for all specials
  CountT f_special = 0.0;
  for (fi=flavors.begin(); fi!=flavors.end(); ++fi) {
    f_special += f_text(*fi);
  }
  if (f_special == 0.0) return; //-- nothing to discount

  //-- discount specials
  for (fi=flavors.begin(); fi!=flavors.end(); ++fi) {
    LexfreqTokTable::iterator wi = lftable.find(*fi);
    if (wi == lftable.end()) continue;

    CountT &f_w = wi->second.count;
    f_w         = f_w / f_special * zf_special;

    for (LexfreqSubtable::iterator wti = wi->second.freqs.begin(); wti != wi->second.freqs.end(); ++wti) {
      LexfreqTagTable::iterator ti = tagtable.find(wti->first);
      if (ti==tagtable.end()) continue;
      CountT &f_wt = wti->second;
      CountT &f_t  = ti->second;
      CountT zf_wt = f_wt / f_special * zf_special;
      f_t      -= (f_wt - zf_wt);
      f_wt      = zf_wt;
    }
  }

  n_tokens -= (f_special - zf_special);
}

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
      if (taster && taster->has_label(lfti->first)) continue; //-- ignore special pseudo-tokens
      n += lfti->second.freqs.size();
    }
  return n;
};

/*----------------------------------------------------------------------
 * I/O
 *----------------------------------------------------------------------*/

//--------------------------------------------------------------
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

//--------------------------------------------------------------
bool mootLexfreqs::load(FILE *file, const char *filename)
{
  mootLexfreqsCompiler lfcomp;
  lfcomp.srcname = filename;
  lfcomp.lexfreqs  = this;
  bool rc = (lfcomp.parse_from_file(file) != NULL);
  lfcomp.srcname = NULL;
  return rc;
}

//--------------------------------------------------------------
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

//--------------------------------------------------------------
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

