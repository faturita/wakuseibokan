#include "stdio.h"
#include "math.h"

int main(int argc, char *argv[])
{
    srand(atoi(argv[1]));


    for(int i=0;i<38;i++)
    {
        float x = (rand() % 3550 + 1); x -= 1800;
        float z = (rand() % 3550 + 1); z -= 1800;

        double rfs = rand();

        printf ("rand %10.5f   x %10.5f ,z %10.5f\n", rfs, x, z);
    }
}
