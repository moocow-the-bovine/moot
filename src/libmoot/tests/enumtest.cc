#include <stdio.h>
#include <string>
#include "mootEnum.h"

/**
 * intuitive version: works now (with -ftemplate-depth-NN, NN>=19)
 */
typedef mootEnum<string, hash<string>, equal_to<string> > stringEnum;

/**
 * almost as intuitive : also requires -ftemplate-depth-NN, NN>=19)
 */
//class stringEnum : public mootEnum<string, hash<string>, equal_to<string> > {};


/**
 * preliminary typedefs? nope.
typedef hash<string> str_hash;
typedef equal_to<string> str_equal;
typedef mootEnum<string, str_hash, str_equal> stringEnum;
*/

/**
 * alternate: bonks
typedef mootEnum<string,hash_map<string,mootEnumID> > stringEnum;
 */

/**
 * Support class : also nope
class stringEnumUtils {
public:
  struct HashFcn {
  public:
    inline size_t operator()(const string &x) const {
      size_t hv = 0;
      for (string::const_iterator xi = x.begin(); *xi != '\0' && xi != x.end(); xi++) {
	hv += (hv<<5)-hv + *xi;
      }
      return hv;
    }
  };
  struct EqualFcn {
  public:
    inline size_t operator()(const string &x, const string &y) const {
      return x==y;
    };
  };
};
typedef mootEnum<string,stringEnumUtils::HashFcn,stringEnumUtils::EqualFcn> stringEnum;
*/



/*
 * MAIN
 */
int main (int argc, char **argv)
{
  stringEnum se;
  //mootStringEnum se;

  while (--argc) {
    fprintf(stderr, "arg: %s\n", *(++argv));
    se.insert(string(*argv));
  }

  for (unsigned i = 0; i < se.ids2names.size(); i++) {
    fprintf(stderr, "id2name(%u)='%s'\t;\tname2id('%s')=%u\n",
	    i, se.id2name(i).c_str(),
	    se.id2name(i).c_str(), se.name2id(se.id2name(i)));
  }

  fprintf(stderr, "\nname2id('bonk')=%u\n", se.name2id("bonk"));
  

  return 0;
}
