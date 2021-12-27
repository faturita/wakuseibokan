#include "telemetry.h"

struct LogStructure {
    float pos1;
    float pos2;
    float pos3;
    float r1;
    float r2;
    float r3;
    float r4;
    float r5;
    float r6;
    float r7;
    float r8;
    float r9;
    float r10;
    float r11;
    float r12;

};

extern int sockfd;
extern struct sockaddr_in servaddr;

void inittelemetry()
{
    /* Clean up */
    bzero(&servaddr, sizeof(servaddr));

    /* Initialize the client to connect to the server on local port 4500 */
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(4500);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    /* Bring up the client socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
}

void logudp(LogStructure logstructure, int sockfd, const SA *pservaddr, socklen_t servlen)
{
    // @NOTE: The socket must be already connected at this point.
    sendto(sockfd, &logstructure, sizeof(logstructure), 0, pservaddr, servlen);
}


void telemetryme(float *dBodyPosition, float *dBodyRotation)
{
    LogStructure logstructure;

    logstructure.pos1 = dBodyPosition[0];
    logstructure.pos2 = dBodyPosition[1];
    logstructure.pos3 = dBodyPosition[2];
    logstructure.r1 = dBodyRotation[0];
    logstructure.r2 = dBodyRotation[1];
    logstructure.r3 = dBodyRotation[2];
    logstructure.r4 = dBodyRotation[3];
    logstructure.r5 = dBodyRotation[4];
    logstructure.r6 = dBodyRotation[5];
    logstructure.r7 = dBodyRotation[6];
    logstructure.r8 = dBodyRotation[7];
    logstructure.r9 = dBodyRotation[8];
    logstructure.r10 = dBodyRotation[9];
    logstructure.r11 = dBodyRotation[10];
    logstructure.r12 = dBodyRotation[11];

    logudp(logstructure, sockfd, (SA *) &servaddr, sizeof(servaddr));

}
