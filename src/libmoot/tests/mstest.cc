#include <stdio.h>
#include <map>
#include <set>

using namespace std;

typedef map<int,set<int> > ISImap;

int main (int argc, char **argv)
{
  ISImap im;

  im[0].insert(0);

  im[1].insert(0);
  im[1].insert(1);

  im[2].insert(2);
  im[2].insert(1);
  im[2].insert(0);

  for (ISImap::const_iterator imi = im.begin(); imi != im.end(); imi++) {
    printf("%d => {", imi->first);
    for (set<int>::const_iterator imii = imi->second.begin(); imii != imi->second.end(); imii++) {
      printf("%s %d", imii == imi->second.begin() ? "" : ",", *imii);
    }
    printf(" }\n");
  }

  return 0;
};
