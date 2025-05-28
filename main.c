#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

__attribute__((optimize(0)))
int foo_v1(int a)
{
        return a + 1;
}

__attribute__((optimize(0)))
int foo(int a)
{
        return 0;
}

#include "patch_function.c"

int main(int argc, char *argv[])
{
        int err;
        unsigned char unpatch_code[PATCH_SIZE];
        
        err = patch_function(foo, foo_v1, unpatch_code);
        if (err) {
                fprintf(stderr, "Failed to patch function foo\n");
                return err;
        }

        printf("%d\n", foo(100));

        err = unpatch_function(foo, unpatch_code);
        if (err) {
                fprintf(stderr, "Failed to unpatch function foo\n");
                return err;
        }

        printf("%d\n", foo(100));

        return 0;
}