#include <stdio.h>

class aClass {
public:
  int ax;
  aClass(int x=0) : ax(x) {};
  virtual ~aClass(void) {};

  virtual void foo(void) {
    printf("aClass::foo() called [ax=%d]\n", ax);
  };
};

class bClass : virtual public aClass {
public:
  int ax;

  bClass(int x=0) : aClass(x) {};
  virtual ~bClass(void) {};

  virtual void foo(void) {
    printf("bClass::foo() called [ax=%d, a::ax=%d]\n", ax, aClass::ax);
  }
};

int main (void) 
{
  aClass *ac = new aClass(1);
  bClass *bc = new bClass(2);
  aClass *abc = new bClass(3);

  ac->foo();
  bc->foo();
  abc->foo();

  return 0;
}
