#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Unit tests.
 *
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[])
{
    char cmd[256];
    int nTests = 59;
    int tests[nTests];
    memset(tests,-1,sizeof(int));

    for(int i=0;i<nTests;i++)
    {
        sprintf(cmd, "./test -test %d",i+1);
        tests[i] = system(cmd);
    }

    for(int i=0;i<nTests;i++)
    {
        printf("Test:%2d\tResult:%s\n", i, ((tests[i]!=-1) ?"Ok" : "ERROR"));
    }

    return 1;
}
