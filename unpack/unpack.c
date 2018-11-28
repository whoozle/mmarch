#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "mmarch_posix.h"

int main(int argc, char ** argv)
{
    struct mmarch_posix_context context;
    mmarch_posix_context_init_default(&context);
    exit(0);
}