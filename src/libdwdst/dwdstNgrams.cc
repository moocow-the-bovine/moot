/* -*- Mode: C++ -*- */

/*============================================================================
 * File: dwdstNgrams.cc
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *    Class for storage & retrieval of N-Gram counts
 *============================================================================*/

#include <stdio.h>
#include <errno.h>

#include <dwdstNgrams.h>
#include <dwdstNgramsCompiler.h>

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
bool dwdstNgrams::load(char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "dwdstNgrams::load(): open failed for file '%s': %s\n",
	    filename,
	    strerror(errno));
    return 0;
  }
  bool rc = load(file);
  fclose(file);
  return rc;
}

bool dwdstNgrams::load(FILE *file, char *filename)
{
  dwdstNgramsCompiler ngcomp;
  ngcomp.srcname = filename;
  ngcomp.ngrams  = this;
  return (ngcomp.parse_from_file(file) != NULL);
}

bool dwdstNgrams::save(char *filename)
{
  FILE *file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "dwdstNgrams::save(): open failed for file '%s': %s\n",
	    filename,
	    strerror(errno));
    return 0;
  }
  bool rc = save(file);
  fclose(file);
  return rc;
}

bool dwdstNgrams::save(FILE *file, char *filename)
{
  set<NgramString> ngrams;

  //-- prepare sorted key-list
  for (NgramStringTable::const_iterator ngti = ngtable.begin(); ngti != ngtable.end(); ngti++) {
    ngrams.insert(ngti->first);
  }

  //-- and output
  NgramString::const_iterator ngii;
  for (set<NgramString>::const_iterator ngi = ngrams.begin(); ngi != ngrams.end(); ngi++) {
    if (ngi->empty()) continue;
    for (ngii = ngi->begin(); ngii != ngi->end(); ngii++) {
      fprintf(file, "%s\t", ngii->c_str());
    }
    fprintf(file, "%g\n", ngtable[*ngi]);
  }

  return 1;
}
