#include <moot.h>
#include <stdio.h>

using namespace moot;
using namespace std;

typedef struct {
  std::string s;
} myData;

int main(void) {
  mootToken tok("foo");
  tok.tok_besttag = "FOO";
  tok.tok_data = new myData();
  ((myData*)tok.tok_data)->s = "blubber";

  printf("text=%s, tag=%s, data=%s\n", tok.tok_text.c_str(), tok.tok_besttag.c_str(), ((myData*)tok.tok_data)->s.c_str());
  return 0;
}
