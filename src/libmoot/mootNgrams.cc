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
 * Information
 *----------------------------------------------------------------------*/
size_t mootNgrams::n_bigrams(void)
{
  size_t n = 0;
  for (NgramTable::const_iterator ngi1 = ngtable.begin(); ngi1 != ngtable.end(); ngi1++)
    {
      n += ngi1->second.freqs.size();
    }
  return n;
}

size_t mootNgrams::n_trigrams(void)
{
  size_t n = 0;
  for (NgramTable::const_iterator ngi1 = ngtable.begin(); ngi1 != ngtable.end(); ngi1++)
    {
      for (BigramTable::const_iterator ngi2 = ngi1->second.freqs.begin();
	   ngi2 != ngi1->second.freqs.end();
	   ngi2++)
	{
	  n += ngi2->second.freqs.size();
	}
    }
  return n;
}

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
  NgramTable::const_iterator ngi1;
  BigramTable::const_iterator ngi2;
  TrigramTable::const_iterator ngi3;

  for (ngi1 = ngtable.begin(); ngi1 != ngtable.end(); ngi1++) {
    const mootTagString &tag1   = ngi1->first;
    const CountT        &count1 = ngi1->second.count;
    fprintf(file, "%s\t%g\n", tag1.c_str(), count1);

    for (ngi2 = ngi1->second.freqs.begin(); ngi2 != ngi1->second.freqs.end(); ngi2++) {
      const mootTagString &tag2   = ngi2->first;
      const CountT        &count2 = ngi2->second.count;

      if (compact) {
	fputc('\t', file);
      } else {
	fprintf(file, "%s\t", tag1.c_str());
      }
      fprintf(file, "%s\t%g\n", tag2.c_str(), count2);

      for (ngi3 = ngi2->second.freqs.begin(); ngi3 != ngi2->second.freqs.end(); ngi3++) {
	const mootTagString &tag3   = ngi3->first;
	const CountT        &count3 = ngi3->second;

	if (compact) {
	  fputs("\t\t", file);
	} else {
	  fprintf(file, "%s\t%s\t", tag1.c_str(), tag2.c_str());
	}
	fprintf(file, "%s\t%g\n", tag3.c_str(), count3);
      }
    }
  }

  return 1;
}

moot_END_NAMESPACE
