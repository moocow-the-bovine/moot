/* -*- Mode: C++ -*- */

/*============================================================================
 * File: dwdstLexfreqs.cc
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *    Class for storage & retrieval of lexical frequency parameters
 *============================================================================*/

#include <stdio.h>
#include <errno.h>

#include <dwdstLexfreqs.h>
#include <dwdstLexfreqsCompiler.h>

DWDST_BEGIN_NAMESPACE

/*----------------------------------------------------------------------
 * Manipulators
 *----------------------------------------------------------------------*/

void dwdstLexfreqs::clear(void)
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
bool dwdstLexfreqs::load(char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "dwdstLexfreqs::load(): open failed for file '%s': %s\n",
	    filename,
	    strerror(errno));
    return 0;
  }
  bool rc = load(file);
  fclose(file);
  return rc;
}

bool dwdstLexfreqs::load(FILE *file, char *filename)
{
  dwdstLexfreqsCompiler lfcomp;
  lfcomp.srcname = filename;
  lfcomp.lexfreqs  = this;
  return (lfcomp.parse_from_file(file) != NULL);
}

bool dwdstLexfreqs::save(char *filename)
{
  FILE *file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "dwdstLexfreqs::save(): open failed for file '%s': %s\n",
	    filename,
	    strerror(errno));
    return 0;
  }
  bool rc = save(file);
  fclose(file);
  return rc;
}

bool dwdstLexfreqs::save(FILE *file, char *filename)
{
  set<LexfreqKey> keys;

  //-- prepare sorted key-list
  for (LexfreqStringTable::const_iterator lfti = lftable.begin(); lfti != lftable.end(); lfti++) {
    keys.insert(lfti->first);
  }

  //-- iterate through sorted keys
  dwdstTokString lasttok;
  for (set<LexfreqKey>::const_iterator keyi = keys.begin(); keyi != keys.end(); keyi++) {
    const dwdstTokString &curtok = keyi->first;
    const dwdstTagString &curtag = keyi->second;

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

DWDST_END_NAMESPACE

