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

/*----------------------------------------------------------------------
 * Manipulators
 *----------------------------------------------------------------------*/

void dwdstLexfreqs::clear(void)
{
  for (LexfreqStringTable::iterator ti = lftable.begin(); ti != lftable.end(); ti++) {
    if (ti->second != NULL) {
      ti->second->clear();
      delete ti->second;
      ti->second = NULL;
    }
  }
  lftable.clear();
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
  set<dwdstTokString> tokens;

  //-- prepare sorted token-list
  for (LexfreqStringTable::const_iterator lfti = lftable.begin(); lfti != lftable.end(); lfti++) {
    tokens.insert(lfti->first);
  }

  //-- iterate through sorted tokens
  set<dwdstTagString> tags;
  LexfreqSubtable    *subt;
  LexfreqCount       total;
  for (set<dwdstTokString>::const_iterator toki = tokens.begin(); toki != tokens.end(); toki++) {
    //-- prepare sorted tag-list
    tags.clear();
    total = 0;
    subt = lftable[*toki];
    if (subt==NULL || subt->empty()) continue;
    for (LexfreqSubtable::const_iterator sti = subt->begin(); sti != subt->end(); sti++) {
      tags.insert(sti->first);
      total += sti->second;
    }

    //-- finally, output
    fprintf(file, "%s\t%g", toki->c_str(), total);
    for (set<dwdstTagString>::const_iterator tagi = tags.begin(); tagi != tags.end(); tagi++) {
      fprintf(file, "\t%s\t%g", tagi->c_str(), (*subt)[*tagi]);
    }
    fputc('\n', file);
  }

  return 1;
}
