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
 * File: mootClassfreqs.cc
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *    Class for storage & retrieval of lexical-class frequency parameters
 *============================================================================*/

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <mootClassfreqs.h>
#include <mootClassfreqsCompiler.h>

moot_BEGIN_NAMESPACE

/*----------------------------------------------------------------------
 * Manipulators
 *----------------------------------------------------------------------*/

void mootClassfreqs::clear(void)
{
  lctable.clear();
  tagtable.clear();
  totalcount = 0;
}

/*----------------------------------------------------------------------
 * Lookup
 *----------------------------------------------------------------------*/

// (inlined)

/*----------------------------------------------------------------------
 * Information
 *----------------------------------------------------------------------*/
size_t mootClassfreqs::n_pairs(void)
{
  size_t n = 0;
  //-- iterate over all classes
  for (ClassfreqTable::const_iterator lcti = lctable.begin();
       lcti != lctable.end();
       lcti++)
    {
      n += lcti->second.freqs.size();
    }
  return n;
};

size_t mootClassfreqs::n_impossible(void)
{
  size_t n = 0;
  //-- iterate over all classes
  for (ClassfreqTable::const_iterator lcti = lctable.begin();
       lcti != lctable.end();
       lcti++)
    {
      for (ClassfreqSubtable::const_iterator lcsi = lcti->second.freqs.begin();
	   lcsi != lcti->second.freqs.end();
	   lcsi++)
	{
	  if (lcti->first.find(lcsi->first) == lcti->first.end())
	    n++;
	    //n += lcti->second.freqs.size();
	}
    }
  return n;
};

/*----------------------------------------------------------------------
 * I/O
 *----------------------------------------------------------------------*/
bool mootClassfreqs::load(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "mootClassfreqs::load(): open failed for file '%s': %s\n",
	    filename,
	    strerror(errno));
    return 0;
  }
  bool rc = load(file,filename);
  fclose(file);
  return rc;
}

bool mootClassfreqs::load(FILE *file, const char *filename)
{
  mootClassfreqsCompiler lccomp;
  lccomp.srcname  = strdup(filename);
  lccomp.cfreqs  = this;
  bool rc = (lccomp.parse_from_file(file) != NULL);
  free(lccomp.srcname);
  lccomp.srcname = NULL;
  return rc;
}

bool mootClassfreqs::save(const char *filename)
{
  FILE *file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "mootClassfreqs::save(): open failed for file '%s': %s\n",
	    filename,
	    strerror(errno));
    return 0;
  }
  bool rc = save(file);
  fclose(file);
  return rc;
}

bool mootClassfreqs::save(FILE *file, const char *filename)
{
  //-- iterate through table
  for (ClassfreqTable::const_iterator lcti = lctable.begin(); lcti != lctable.end(); lcti++) {
    const LexClass         &lclass = lcti->first;
    const ClassfreqEntry   &entry  = lcti->second;
    //-- print class
    for (LexClass::const_iterator lci = lclass.begin(); lci != lclass.end(); lci++) {
      if (lci != lclass.begin()) fputc(' ', file);
      fputs(lci->c_str(), file);
    }
    fprintf(file, "\t%g", entry.count);

    for (ClassfreqSubtable::const_iterator ei = entry.freqs.begin(); ei != entry.freqs.end(); ei++) {
      fprintf(file, "\t%s\t%g", ei->first.c_str(), ei->second);
    }
    fputc('\n', file);
  }
  return 1;
}

moot_END_NAMESPACE

