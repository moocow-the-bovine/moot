#include <stdio.h>
#include <string>

#include "dwdstTagger.h"
#include <FSMSymSpec.h>

/*
const FSMSymbolString &dwdstTagger::dequote_tag_string(const FSMSymbolString &str) {
  char clast = '\0';
  dequoted_str.clear();
  if (str.size() >= dequoted_str.capacity()) dequoted_str.reserve(str.size());
  
  for (FSMSymbolString::const_iterator si = str.begin(); si != str.end(); si++) {
    switch (clast) {
    case '\0':
      if (*si == '[') {
	clast = '[';
      } else {
	//clast = '\0';
	dequoted_str.push_back(*si);
      }
      break;
      
    case '[':
      if (*si == '_') {
	clast = '_';
	break;
      }
      clast = '_';
    case '_':
      if (*si == ']') {
	clast = ']';
      } else {
	clast = '_';
	dequoted_str.push_back(*si);
      }
      break;
      
    case ']':
      if (*si == '[') {
	dequoted_str.push_back('.');
	clast = '[';
      } else {
	dequoted_str.push_back(*si);
	clast = '\0';
      }
      break;
	
    default:
      clast = '\0';
      dequoted_str.push_back(*si);
    }
  }
  return dequoted_str;
}
*/

int main (int argc, char **argv) {
  string str;
  dwdstTagger tgr;
  while (--argc > 0) {
    ++argv;
    str = string(*argv);
    printf("dequote('%s') = %s\n", str.c_str(), tgr.dequote_tag_string(str).c_str());
  }
  return 0;
}
