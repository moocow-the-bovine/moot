#include <stdio.h>
#include <list>

using namespace std;
typedef list<size_t> myList;

const size_t nmax = 1048576;

void dolist(size_t niters)
{
  myList *l = new myList;
  for (size_t i=0; i<niters; i++) { l->push_back(i); }
  //l->clear();
  delete l;
}

int main (void)
{
  fprintf(stderr, "(PRE): Check memory usage ? "); fgetc(stdin);
  dolist(nmax*1);

  fprintf(stderr, "(POST-1): Check memory usage ? "); fgetc(stdin);
  dolist(nmax*2);

  fprintf(stderr, "(POST-2): Check memory usage ? "); fgetc(stdin);
  dolist(nmax*3);

  return 0;
}
