#include <stdio.h>

class Aclass {
public:
  //-- typedefs
  typedef float ProbT;

public:
  //-- data
  ProbT p;

public:
  //-- methods
  Aclass(ProbT prob=0) : p(prob)
  {
    printf("Aclass::Aclass(%g) called.\n", p);
  };
  ~Aclass(void) {
    printf("Aclass::~Aclass() called (p=%g).\n", p);
  };

  ProbT testme(void) {
    return p;
  };
};


class Bclass : public Aclass {
public:
  //-- typedefs
  //typedef float ProbT; // inherited

public:
  //-- data
  ProbT p;

public:
  //-- methods
  Bclass(ProbT prob=0) : p(prob)
  {
    printf("Bclass::Bclass(%g) called.\n", p);
  };

  ~Bclass(void) {
    printf("Bclass::~Bclass() called (p=%g).\n", p);
  };

  ProbT testme(void) {
    return p;
  };
};


int main (int argc, char **argv)
{
  char *progname = *argv;

  printf("%s: creating new Aclass(1):\n", progname);
  Aclass *ac = new Aclass(1);

  printf("%s: creating new Bclass(2):\n", progname);
  Bclass *bc = new Bclass(2);
  printf("\n");

  printf("%s: Aclass(1)->testme()=%g\n", progname, ac->testme());
  printf("%s: Bclass(2)->testme()=%g\n", progname, bc->testme());
  printf("\n");


  printf("%s: deleting Aclass(1):\n", progname);
  delete ac;

  printf("%s: deleting Bclass(1):\n", progname);
  delete bc;

  return 0;
}
