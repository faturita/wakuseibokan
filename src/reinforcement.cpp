/**
 *
 * Reinforcement Learning and Dynamic Programming
 *
 * [NPC Decision Making Learning with Randomness]
 * [Sutton,Barto, Reinforcement Learning]
 *
 * This is the Maze:
 *
 * |     ||     ||     ||     ||     ||  >  |
 * |     ||  X  ||  X  ||     ||     ||     |
 * |     ||  X  ||  X  ||     ||  D  ||     |
 * |     ||     ||     ||  M  ||     ||     |
 *
 * D is Danger, M is the guy and > is where the exit is located.
 * X are places where the character cannot go.
 *
 ***/


#include "carrier.h"


#define NORTH 1
#define SOUTH 3
#define EAST 0
#define WEST 2

#define LEFT 2
#define RIGHT 0

#define DOWN 3
#define UP 1

/**
 * @brief Get the correct value of the V-value function considering the maze shape. Walls are bounced.
 * @param V
 * @param oj
 * @param oi
 * @param j
 * @param i
 * @return
 */
float get(float V[4][6],int oj, int oi,int j, int i)
{
    // These are the usual maze boundaries.
    if (j<=0)
        j=0;
    if (i<=0)
        i=0;
    if (j>3)
        j=3;
    if (i>5)
        i=5;

    // These are the places where the isle is located.
    if (j==1 && i==1) {j=oj;i=oi;}
    if (j==1 && i==2) {j=oj;i=oi;}
    if (j==2 && i==1) {j=oj;i=oi;}
    if (j==2 && i==2) {j=oj;i=oi;}

    return V[j][i];
}

void showV(float V[4][6])
{
    for(int i=0;i<4;i++)
    {
        for (int j=0;j<6;j++)
        {
            printf("|  ");
            printf("%6.2f",V[i][j]);
            printf("  |");
        }
        printf("\n");
    }
}
void showS(int S[4][6])
{
    for(int i=0;i<4;i++)
    {
        for (int j=0;j<6;j++)
        {
            printf("|  ");
            if (S[i][j] == 1)
                printf("M");
            else if (S[i][j] == -1)
                printf("D");
            else if (S[i][j] == 2)
                printf("X");
            else if (S[i][j] == 3)
                printf(">");
            else
                printf(" ");
            printf("  |");

        }
        printf("\n");
    }
}
int main(int argc, char*argv[])
{
    int S[4][6];

    float R[4][6];

    float Pss[4][6][4][4][6];

    float V[4][6];

    float Vs[4][6];

    memset(S,0,sizeof(int)*4*6);

    S[2][4] = -1;
    S[0][5] = 3;
    S[3][3] = 1;

    S[1][1] = 2;
    S[1][2] = 2;
    S[2][1] = 2;
    S[2][2] = 2;

    showS(S);

    memset(R,1,sizeof(float)*4*6);

    for(int j=0;j<4;j++) for (int i=0;i<6;i++) R[j][i] = 1;

    R[2][4] = 100;
    R[0][5] = 0;

    showV(R);

    memset(Pss,0,sizeof(float)*4*6*4*4*6);

    for(int j=0;j<4;j++)
    {

        for(int i=0;i<6;i++)
        {

            Pss[j][i][0][j][i] = 0.25;
            Pss[j][i][1][j][i] = 0.25;
            Pss[j][i][2][j][i] = 0.25;
            Pss[j][i][3][j][i] = 0.25;

            if ((i+1)<6)
                Pss[j][i][0][j][i+1] = 0.5;
            if (j+1<4)
                Pss[j][i][3][j+1][i] = 0.5;

            if (i-1>=0) Pss[j][i][2][j][i-1] = 0.5;
            if (j-1>=0) Pss[j][i][1][j-1][i] = 0.5;

            if (j+1<4 ) Pss[j][i][0][j+1][i] = 0.25;
            if (j-1>=0) Pss[j][i][0][j-1][i] = 0.25;

            if (i-1>=0) Pss[j][i][3][j][i-1] = 0.25;
            if (i+1<6 ) Pss[j][i][3][j][i+1] = 0.25;

            if (j-1>=0) Pss[j][i][2][j-1][i] = 0.25;
            if (j+1<4)  Pss[j][i][2][j+1][i] = 0.25;

            if (i-1>=0) Pss[j][i][1][j][i-1] = 0.25;
            if (i+1<6)  Pss[j][i][1][j][i+1] = 0.25;

        }

    }



                           Pss[0][1][3][1][1] = 1000000;Pss[0][2][3][1][2] = 1000000;
    Pss[1][0][0][1][1] = 10000;                                             ;Pss[1][3][2][1][2] = 1000000;
    Pss[2][0][0][2][1] = 10000;                                             ;Pss[2][3][2][2][2] = 1000000;
                           Pss[3][1][1][2][1] = 1000000;Pss[3][2][1][2][2] = 1000000;

    memcpy(V,R,sizeof(float)*4*6);

    for(int k=0;k<200;k++)
    {
    memcpy(Vs,V,sizeof(float)*4*6);
    for(int j=0;j<4;j++)
    {
        for (int i=0;i<6;i++)
        {
            if (j==1 && i==1) continue;
            if (j==1 && i==2) continue;
            if (j==2 && i==1) continue;
            if (j==2 && i==2) continue;

            if (j==0 && i==5) continue;

            // Compute U for each cell.
            float U[4];
            memset(U,0,sizeof(float)*4);

            U[DOWN] =  (  0.5*get(V,j,i,j+1,i) + 0.25*get(V,j,i,j,i-1) + 0.25*get(V,j,i,j,i+1));
            U[UP]   =  (  0.5*get(V,j,i,j-1,i) + 0.25*get(V,j,i,j,i-1) + 0.25*get(V,j,i,j,i+1));
            U[LEFT] =  (  0.5*get(V,j,i,j,i-1) + 0.25*get(V,j,i,j-1,i) + 0.25*get(V,j,i,j+1,i));
            U[RIGHT]=  (  0.5*get(V,j,i,j,i+1) + 0.25*get(V,j,i,j-1,i) + 0.25*get(V,j,i,j+1,i));

            if (j==0) U[NORTH] = 1000000;
            if (i==0) U[LEFT] = 1000000;
            if (j==3) U[DOWN] = 1000000;
            if (i==5) U[RIGHT] = 1000000;


            if (j==1 && i==3) U[LEFT] = 100000;
            if (j==2 && i==3) U[LEFT] = 100000;

            if (j==1 && i==0) U[RIGHT] = 100000;
            if (j==2 && i==0) U[RIGHT] = 100000;

            if (j==0 && i==1) U[SOUTH] = 100000;
            if (j==0 && i==2) U[SOUTH] = 100000;

            if (j==3 && i==1) U[NORTH] = 100000;
            if (j==3 && i==2) U[NORTH] = 100000;

            // Choose the smallest U
            float Us=U[0];
            int min=0;
            for(int a=1;a<4;a++)
            {
                if (U[a]<Us)
                {
                    Us = U[a];
                    min=a;
                }
            }

            // Set the cell's value to the smallest action plus the original cell cost.
            Vs[j][i] = Us + R[j][i];

            printf("%d,%d: %10.8f\n",j,i, Vs[j][i]);

            //exit(-1);

        }
    }

    printf("----------------------------------------\n");
    memcpy(V, Vs, sizeof(float)*6*4);
    showV(V);
    }

    printf("----------------------------------------\n");showV(V);

    return 1;



}
