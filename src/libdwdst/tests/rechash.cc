#include <hash_map>
#include <stdio.h>

#include "dwdstTypes.h"
#include "pairtest.h"

/* recursive version */
typedef dwdstTokString                         LexfreqKey;
typedef dwdstTagString                         LexfreqSubkey;
typedef float                                  LexfreqCount;
typedef hash_map<LexfreqSubkey,LexfreqCount>   LexfreqSubtable;
typedef hash_map<LexfreqKey,LexfreqSubtable *> LexfreqTable;

LexfreqTable t;
dwdstTokString tok = "foo";
dwdstTagString tag = "NN";
LexfreqCount lfcount = 42;


void test_insert(void) {
  LexfreqTable::iterator ti = t.find(tok);
  if (ti != t.end()) {
    if (!ti->second) ti->second = new LexfreqSubtable;
  } else {
    ti = t.insert(pair<LexfreqKey,LexfreqSubtable *>(tok,new LexfreqSubtable)).first;
  }
  LexfreqSubtable::iterator sti = ti->second->find(tag);
  if (sti != ti->second->end()) {
    sti->second += lfcount;
  } else {
    sti = ti->second->insert(pair<dwdstTagString,LexfreqCount>(tag,lfcount)).first;
  }
}

LexfreqCount test_retrieve(void) {
  LexfreqTable::const_iterator ti = t.find(tok);
  if (ti == t.end() || ti->second == NULL) return 0;

  LexfreqSubtable::const_iterator sti = ti->second->find(tag);
  if (sti != ti->second->end()) return 0;
  return sti->second;
}

int main (void) {
  //-- expensive
  /*--ok
    //-- insert
    t[tok] = new LexfreqSubtable;
    (*t[tok])[tag] = lfcount;
    //-- retrieve
    printf("t[%s][%s] = %g\n", tok.c_str(), tag.c_str(), (*t[tok])[tag]);
  */


  //-- less expensive (?), much uglier
  //-- insert
  LexfreqTable::iterator ti = t.find(tok);
  if (ti != t.end()) {
    if (!ti->second) ti->second = new LexfreqSubtable;
  } else {
    ti = t.insert(pair<LexfreqKey,LexfreqSubtable *>(tok,new LexfreqSubtable)).first;
  }
  LexfreqSubtable::iterator sti = ti->second->find(tag);
  if (sti != ti->second->end()) {
    sti->second += lfcount;
  } else {
    sti = ti->second->insert(pair<dwdstTagString,LexfreqCount>(tag,lfcount)).first;
  }
  LexfreqCount count_out = sti->second;

  //-- retrieve (dangerous)
  printf("t[%s][%s] = %g (out=%g)\n", tok.c_str(), tag.c_str(), (*t[tok])[tag], count_out);

  //-- test
  printf("Benchmarking %d iterations of test_%s()...\n",
	 TEST_COUNT,
	 TEST_NAME
	 );
  for (int i = 0; i < TEST_COUNT; i++) {
    TEST_FCN();
  }

  return 0;
}
