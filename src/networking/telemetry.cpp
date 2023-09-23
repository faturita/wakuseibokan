/* ============================================================================
**
** Telemetry
**
** This code sends telemetry information to any client that is UDP listening.
** The information that is being sent is unique per unit (comes from Vehicle)
** when the telemetry flag is enabled on each unit.
**
** This allow to monitor the behaviour of each one of the vehicles.  Any entity
** can send telemetry information at each simulation step.
**
** ========================================================================= */
#include "telemetry.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "../propertystore.h"
#include "../profiling.h"

extern unsigned long timer;

struct ModelRecord {
    unsigned long recordtimer;
    int number;
    int health;
    int power;
    float bearing;

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

struct Connection
{
    int sockfd;
    struct sockaddr_in servaddr;
};

std::vector<Connection> connections;

Connection addNewTelemetryListener(char ip[], int port)
{
    Connection connection;

    /* Clean up */
    bzero(&connection.servaddr, sizeof(connection.servaddr));

    /* Initialize the client to connect to the server on local port 4500 */
    connection.servaddr.sin_family = AF_INET;
    connection.servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &connection.servaddr.sin_addr);

    /* Bring up the client socket */
    connection.sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    return connection;
}

void inittelemetry()
{
    // @NOTE: The info is being read from a configuration file.
    // (who has joined the game)
    //connections.push_back(addNewTelemetryListener("127.0.0.1",4500));

    char filename[256];
    sprintf(filename,"%s%s%s","conf",DIRSEPARATOR,"telemetry.endpoints.ini");
    PropertyStore ps(filename);

    //ps.Set("client","127.0.0.1");

    //ps.Save();

    ps.Load();

    char *endpoints = ps.Get("endpoints");

    int iendpoints = atoi(endpoints);

    dout << iendpoints << std::endl;

    for(int i=0;i<iendpoints;i++)
    {
        char telkey[256];
        char ipkey[256];
        sprintf(telkey,"endpoint#%d",i+1);
        sprintf(ipkey,"port#%d", i+1);
        char *telemetryendpoint = ps.Get(telkey);
        char *portendpoint = ps.Get(ipkey);
        int portnumber = atoi(portendpoint);

        if (strcmp(telemetryendpoint,EMPTYVALUE)!=0 && strcmp(portendpoint,EMPTYVALUE)!=0)
        {
            if (portnumber <=0 || portnumber > 65500)
                portnumber = 4500;

            dout << "Hooking up to telemetry endpoint at " << telemetryendpoint << ":" << portendpoint << std::endl;
            connections.push_back(addNewTelemetryListener(telemetryendpoint,portnumber));
        }
    }
}


void telemetryme(int number, int health, int power, float bearing, float *dBodyPosition, float *dBodyRotation)
{
    ModelRecord logstructure;

    logstructure.recordtimer = timer;
    logstructure.number = number;
    logstructure.health = health;
    logstructure.power = power;
    logstructure.bearing = bearing;

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


    for (size_t i=0; i<connections.size(); i++) {
        // @NOTE: The socket must be already connected at this point.
        sendto(connections[i].sockfd, &logstructure, sizeof(logstructure), 0, (SA *)&connections[i].servaddr, sizeof(connections[i].servaddr));
    }

}
