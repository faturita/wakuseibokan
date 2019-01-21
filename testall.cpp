#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char cmd[256];
    int tests[10];
    memset(tests,-1,sizeof(int));

    for(int i=0;i<10;i++)
    {
        sprintf(cmd, "./waku -test %d",i+1);
        tests[i] = system(cmd);
    }

    for(int i=0;i<10;i++)
    {
        printf("Test:%2d\tResult:%s\n", i, ((tests[i]!=-1) ?"Ok" : "ERROR"));
    }

    return 1;
}
