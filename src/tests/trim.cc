#include <stdio.h>
#include <string>

using namespace std;

void trim_string1(string *cti) {
    size_t p;
    printf(">> trimming '%s'\n", cti->c_str());

    // -- trim leading spaces
    if ((p = cti->find_first_not_of(" ",0)) != 0) {
	printf(">> first_not_of(' ',0) = %d\n", p);
	cti->erase(0,p);
    } else {
	printf(">> first_not_of(' ',0) = 0 : (not trimmed)\n");
    }

    // -- trim trailing spaces
    if ((p = cti->find_last_not_of(" ")) != cti->size()-1) {
	printf(">> last_not_of(' ') = %d != (size=%u) : (trimmed)\n", p, cti->size());
	cti->erase(p+1, cti->size()-p);
    } else {
	printf(">> last_not_of(' ') = size = %u : (not trimmed)\n", cti->size());
    }
}

void trim_string2(string *cti) {
    // -- trim leading spaces
    cti->erase(0,cti->find_first_not_of(" ",0));

    // -- trim trailing spaces
    cti->erase(cti->find_last_not_of(" ")+1,cti->npos);
}




int main(int argc, char **argv) {
    string s;
    while (--argc > 0) {
	++argv;
	s = *argv;
	//trim_string1(&s);
	trim_string2(&s);
	printf("trim('%s') = '%s'\n", *argv, s.c_str());
    }
    return 0;
}
