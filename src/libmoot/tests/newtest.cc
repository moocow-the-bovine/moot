#include <stdio.h>
#include <list>
#include <string>
#include <set>
#include <sys/time.h>

using namespace std;

class mTokenSTL {
public:
  string text;
  string besttag;
  set<string> analyses;
  void *data;
public:
  mTokenSTL(void)
    : text(""),
      besttag(""),
      data(NULL)
  {};
};

class mTokenPtr {
public:
  string *text;
  string *besttag;
  set<string> *analyses;
  void *data;

  mTokenPtr(void)
    : text(NULL),
      besttag(NULL),
      analyses(NULL),
      data(NULL)
  {};
};

void report(timeval &started,
	    timeval &stopped,
	    size_t niters,
	    const char *name)
{
  double elapsed =
	stopped.tv_sec-started.tv_sec
	+ (double)(stopped.tv_usec-started.tv_usec)/1000000.0;
  printf("%s : %u iters / %.2g sec = %.2g iters/sec\n",
	 name, niters, elapsed, (double)niters/elapsed);
};

#define DOTEST(tbeg,tend,n,CODE) \
  gettimeofday(&tbeg,NULL); \
  for (int i = 0; i < n; i++) { CODE } \
  gettimeofday(&tend,NULL); \
  report(tbeg,tend,n,#CODE);

#define NITERS 1048576

int main(void)
{
  timeval started, stopped;

  DOTEST(started,stopped,NITERS,{mTokenSTL *s = new mTokenSTL(); delete s;});
  DOTEST(started,stopped,NITERS,{mTokenPtr *s = new mTokenPtr(); delete s;});

  return 0;
};
