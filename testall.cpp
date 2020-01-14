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
    int tests[20];
    memset(tests,-1,sizeof(int));

    for(int i=0;i<19;i++)
    {
        sprintf(cmd, "./test -test %d",i+1);
        tests[i] = system(cmd);
    }

    for(int i=0;i<19;i++)
    {
        printf("Test:%2d\tResult:%s\n", i, ((tests[i]!=-1) ?"Ok" : "ERROR"));
    }

    return 1;
}
