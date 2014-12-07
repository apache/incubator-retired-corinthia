#include "DFUnitTest.h"
#include <stdio.h>

extern TestGroup APITests;

TestGroup *groups[] = {
    &APITests,
    NULL
};

int main(int argc, const char **argv)
{
    setbuf(stdout,NULL);
    utrun(groups);
    return 0;
}
