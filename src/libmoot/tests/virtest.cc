#include <stdio.h>

class baseClass {
public:
  baseClass(void) { printf("base::base() called\n"); };
  virtual ~baseClass(void) { printf("base::~base() called\n"); };

  void a(void) { printf("base::a() called\n"); };
  virtual void b(void) { printf("base::b() called\n"); };
  virtual void c(void) { printf("base::c() called\n"); };
};

class derivedClass : public baseClass {
 public:
  virtual ~derivedClass(void) { printf("derived::~derived() called\n"); };

  void a(void) { printf("derived::a() called\n"); };
  void b(void) { printf("derived::b() called\n"); };
  virtual void c(void) { printf("derived::c() called\n"); };
};

class derivedClass2 : public derivedClass {
 public:
  virtual ~derivedClass2(void) { printf("derived2::~derived2() called\n"); };

  //void a(void) { printf("derived2::a() called\n"); };
  void b(void) { printf("derived2::b() called\n"); };
  virtual void c(void) { printf("derived2::c() called\n"); };
};

void test_indirect (baseClass &bas, const char *name="?")
{
  printf("\n----- test_inidirect() %s.*()\n", name);
  bas.a();
  bas.b();
  bas.c();
};

int main (void)
{
  baseClass bas;
  derivedClass der;
  derivedClass2 der2;

  printf("\n----- test bas.*()\n");
  bas.a();
  bas.b();
  bas.c();

  printf("\n----- test der.*()\n");
  der.a();
  der.b();
  der.c();

  test_indirect(bas, "bas");
  test_indirect(der, "der");
  test_indirect(der2, "der2");

  printf("\n----- cleanup\n");
  return 0;
}
