#include <hash_map>
#include <stdio.h>

#include "dwdstTypes.h"
#include "pairtest.h"

/* -- pair version: bummer: no _STL::hash<pair...>() defined */
typedef pair<dwdstTokString,dwdstTagString>  LexfreqKey;
typedef float                                LexfreqCount;

class LexfreqStringUtils {
public:
  struct HashFcn {
  public:
    inline size_t operator()(const LexfreqKey &x) const {
      size_t hv = 0;
      string::const_iterator i;
      for (i = x.first.begin(); i != x.first.end(); i++) {
	hv += (hv<<5)-hv + *i;
      }
      for (i = x.second.begin(); i != x.second.end(); i++) {
	hv += (hv<<5)-hv + *i;
      }
      return hv;
    };
  };//-- HashFcn

  struct EqualFcn {
  public:
    inline size_t operator()(const LexfreqKey &x, const LexfreqKey &y) const {
      return x==y;
    };
  };
};

typedef
  hash_map<LexfreqKey,
	   LexfreqCount,
	   LexfreqStringUtils::HashFcn,
	   LexfreqStringUtils::EqualFcn>
  LexfreqTable;


LexfreqTable t;
dwdstTokString   tok = "foo";
dwdstTagString   tag = "NN";
LexfreqCount lfcount = 42.24;

void test_insert(void) {
  t[LexfreqKey(tok,tag)] = lfcount;
}

LexfreqCount test_retrieve(void) {
  LexfreqTable::const_iterator ti = t.find(LexfreqKey(tok,tag));
  if (ti == t.end()) return 0;
  return ti->second;
}


int main (void) {
  //-- insert
  t[LexfreqKey(tok,tag)] = lfcount;

  //-- retrieve
  printf("t[%s,%s] = %g\n", tok.c_str(), tag.c_str(), t[LexfreqKey(tok,tag)]);

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

