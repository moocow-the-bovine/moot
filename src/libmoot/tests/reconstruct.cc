#include <stdio.h>

class aClass {
public:
  int a_x;
public:
  aClass(int x=0, int flag=0)
    : a_x(x)
  {
    printf("aClass::aClass called: [a_x=%d, flag=%d]\n", a_x, flag);
  };

  virtual ~aClass(void) {
    printf("aClass::~aClass called [a_x=%d]\n", a_x);
  };

  virtual void foo(void) {
    printf("aClass::foo() called [a_x=%d]\n", a_x);
  };
  virtual void bar(void) {
    printf("aClass::bar() called [a_x=%d]\n", a_x);
  };
};

class bClass : public aClass {
public:
  bClass(int x=0, int flag=1)
    : aClass(x,flag)
  {
    printf("bClass::bClass called: [a_x=%d, flag=%d]\n", a_x, flag);
  };

  virtual ~bClass(void) {
    printf("bClass::~bClass called [a_x=%d]\n", a_x);
  };

  virtual void bar(void) {
    printf("bClass::bar() called [a_x=%d]\n", a_x);
  };
};

class cClass {
public:
  aClass a;

  cClass(int i) : a(i) { fprintf(stderr, "cClass::cClass() called [i=%d]\n", i); };
  ~cClass(void) { fprintf(stderr, "cClass::~cClass() called.\n"); };
};


void dotest(void)
{
  aClass *ac;
  bClass *bc;
  cClass *cc;

  ac = new aClass(1);
  bc = new bClass(2);
  cc = new cClass(3);

  ac->foo();
  bc->foo();

  ac->bar();
  bc->bar();

  delete ac;
  delete bc;
  delete cc;
}

void dotest2(void)
{
  cClass cc(3);
}

int main (void)
{
  dotest2();
  return 0;
}
