#include <string>
#include <stdio.h>

using namespace std;

class mystring {
public:
  string s1;
  string s2;

  mystring(void)
  {
    s1=string();
    s2=string();
  };

  mystring(const string &mys1)
  {
    s1=mys1;
    s2=mys1;
  };

  mystring(const string &mys1, const string &mys2)
  {
    s1=mys1;
    s2=mys2;
  };
};


int main(int argc, char **argv) {
  mystring
    ms,
    ms1("foo"),
    ms2("foo","bar");

  printf("ms.s1=%s ; ms.s2=%s\n", ms.s1.c_str(), ms.s2.c_str());
  printf("ms1.s1=%s ; ms1.s2=%s\n", ms1.s1.c_str(), ms1.s2.c_str());
  printf("ms1.s1=%s ; ms1.s2=%s\n", ms2.s1.c_str(), ms2.s2.c_str());

  return 0;
}
