#include <stdio.h>
#include "dwdst_trainer.h"

dwds_tagger_trainer dwdstt;

int main (int argc, char **argv) {
    dwdstt.load_param_file(stdin);
    dwdstt.save_param_file(stdout);
    return 0;
}
