#include <stdio.h>
#include <string>

#include "mootUtils.h"

using namespace std;
using namespace moot;

const char *delim = ", ";

int main (int argc, char **argv) {
  while (--argc > 0) {
    string s = *(++argv);
    list<string> st = moot_strtok(s, delim);

    printf("moot_strtok(%s)=<", s.c_str());
    for (list<string>::const_iterator sti = st.begin(); sti != st.end(); sti++)
      {
	printf(" \"%s\"", sti->c_str());
      }
    printf(" >\n");
  }
  return 0;
}
