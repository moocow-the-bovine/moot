#include <stdio.h>

class aclass {
public:
  typedef float ProbT;
public:
  ProbT p;
  aclass(ProbT x=0) : p(x)
  {
    printf("aclass::aclass(%g) called: sizeof(ProbT)=%u\n", p, sizeof(ProbT));
  };
  ~aclass(void) {};
};

class bclass : public aclass {
public:
  typedef double ProbT ;
public:
  ProbT p;
  bclass(ProbT x=0) : p(x)
  {
    printf("bclass::bclass(%g) called: sizeof(ProbT)=%u\n", p, sizeof(ProbT));
  };
  ~bclass(void) {};
};

int main (void) {
  aclass a(0.42);
  bclass b(420.247);

  return 0;
}
