#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"


int get_N(int argc, char* argv[]);


int main(int argc, char* argv[])
{
    int N = get_N(argc, argv);
    printf("N is %d\n", N);
    return 0;
}

int get_N(int argc, char* argv[])
{
    if (argc < 3) {
        printf("not enough arguments.\n");
        exit(0);
    }
    if (strcmp(argv[1], PROCESS_ARG)) {
        printf("wrong 'process argument' flag.\n");
        exit(0);
    }
    int X = atoi(argv[2]);
    if (1 <= X && X <= 9) {
        return X + 1;
    }
    printf("process argument is out of range.\n");
    exit(0);
}
