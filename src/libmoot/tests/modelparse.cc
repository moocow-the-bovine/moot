#include <mootUtils.h>
#include <string>

using namespace std;
using namespace moot;

int main (int argc, char **argv)
{
  string binfile,lexfile,ngfile,lcfile;
  while (--argc > 0) {
    char *modelname = *++argv;
    printf("\nMODELNAME=`%s'\n", modelname);
    if (!hmm_parse_model_name(modelname,binfile,lexfile,ngfile,lcfile)) {
      fprintf(stderr, "parse_model_name(%s) failed!\n", modelname);
      continue;
    }
    printf("  binfile=`%s'\n", binfile.c_str());
    printf("  lexfile=`%s'\n", lexfile.c_str());
    printf("   ngfile=`%s'\n", ngfile.c_str());
    printf("   lcfile=`%s'\n", lcfile.c_str());
  }
  return 0;
}
