#include <stdio.h>

class aClass {
public:
  virtual ~aClass(void) {};
  inline void testme1(void) {
    printf("aClass::testme1() called... ");
    testme2();
  };
  virtual void testme2(void) {
    printf("aClass::testme2() called\n");
  };
};

class bClass : public aClass {
public:
  virtual ~bClass(void) {};
  inline void testme1(void) {
    printf("bClass::testme1() called... ");
    testme2();
  };
  virtual void testme2(void) {
    printf("bClass::testme2() called\n");
  };
};

int main (void) {
  aClass *ac = new aClass;
  bClass *bc = new bClass;
  aClass *abc = new bClass;

  printf("ac->testme1() : "); ac->testme1();
  printf("bc->testme1() : "); bc->testme1();
  printf("abc->testme1(): "); abc->testme1();

  return 0;
};
