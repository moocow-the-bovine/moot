/*--------------------------------------------------------------------------
 * File: dwdst.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : the guts
 *--------------------------------------------------------------------------*/

#include "dwdst.h"

#include "FSMTypes.h"

#include <set>
#include <list>
//#include <string>

bool dwdst_tag_stream(FILE *in,
		      FILE *out,
		      FSM *morph,
		      FSMSymSpec *syms,
		      bool want_attrs=false)
{
  return true;
}

bool dwdst_tag_token(char *tok,
		     FILE *out,
		     FSM *morph,
		     FSMSymSpec *syms,
		     bool want_attrs=false)
{
  FSM *result;
  FSM *tmp = new FSM();
  FSMSymbolString *s;
  set<FSMStringWeight> results;

  // guts
  tmp->fsm_clear();
  s = new FSMSymbolString(tok);

  result = morph->fsm_lookup(*s,tmp,true);
  result->fsm_strings(syms, &results, false, want_attrs);
  
  fprintf(out,"%s: %d Analyse(n)\n", tok, results.size());
  string *str;
  for (set<FSMStringWeight>::iterator r = results.begin(); r != results.end(); r++) {
    //fprintf(out, "\t%s\n", AnsiString(r->c_str()));
    //fprintf(out, "\t%s\n", r);
    //fprintf(out, "\t%s\n", r->ostr);
    str = new string(r->ostr);
    fprintf(out, "\t%s\n", str);
  }
  fprintf(out,"\n");

  // cleanup
  //delete s;
  //tmp->fsm_free();
  //result->fsm_free();
  //delete tmp;
  //delete result;

  return true;
}
