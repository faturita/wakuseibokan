#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "lobby.h"

// LobbyConnection contains the socket fd and addr of the game clients (acting as servers to receive the model information).
struct LobbyConnection
{
    int modelsockfd;
    struct sockaddr_in modelserveraddr;
};

std::vector<LobbyConnection> lobby;

// Server address and socket that runs on CLIENTs (as servers to receive the model information).
struct sockaddr_in modelserverasserveraddr, cliaddr;
int modelsockasserverfd;

// Controller Server address which is used by both the GAME CLIENT and GAME SERVER as UDP Socket to transmit the commands.
struct sockaddr_in controllerserveraddr;
int controllersockfd;


LobbyConnection newplayer(char ip[], int port)
{
    LobbyConnection con;

    /* Clean up */
    bzero(&con.modelserveraddr, sizeof(con.modelserveraddr));

    /* Initialize the client to connect to the server on local port 4500 */
    con.modelserveraddr.sin_family = AF_INET;
    con.modelserveraddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &con.modelserveraddr.sin_addr);

    /* Bring up the client socket */
    con.modelsockfd = socket(AF_INET, SOCK_DGRAM, 0);

    return con;
}

// Init lobby from the GAME SERVER side (running as client for the UDP connection to stream the model information).
void init_lobby()
{
    // @FIXME: Read the parameters from a configuration file or from some structure with the lobby information
    // (who has joined the game)
    //lobby.push_back(newplayer("127.0.0.1",4500));
    lobby.push_back(newplayer("192.168.1.195",4500));

    printf(" Setting up GAME SERVER to connect as client for the model...\n");
}


// Join the lobby as the GAME CLIENT side (which is a server from the point of view of the model because it receives the data).
void join_lobby()
{
    modelsockasserverfd = socket(AF_INET, SOCK_DGRAM, 0);
    //fcntl(sockfd, F_SETFL, O_NONBLOCK);

    bzero(&modelserverasserveraddr, sizeof(modelserverasserveraddr));
    modelserverasserveraddr.sin_family = AF_INET;
    modelserverasserveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    modelserverasserveraddr.sin_port = htons(4500);

    bind(modelsockasserverfd, (SA *) &modelserverasserveraddr, sizeof(modelserverasserveraddr));

    printf("Setting up the GAME CLIENT to connect as server for the model.\n");
}


void notify(unsigned long timerparam, size_t id, Vehicle *v)
{
    TickRecord tickrecord;

    tickrecord = v->serialize();

    tickrecord.timerparam = timerparam;
    tickrecord.id = id;

    for (size_t i=0; i<lobby.size(); i++) {
        // @NOTE: The socket must be already connected at this point.
        sendto(lobby[i].modelsockfd, &tickrecord, sizeof(tickrecord), 0, (SA *)&lobby[i].modelserveraddr, sizeof(lobby[i].modelserveraddr));
    }
}

// This is called by the GAME CLIENT to receive constantly the model information.
int receive(TickRecord *record)
{
    socklen_t len;
    SA pcliaddr;

    socklen_t clilen=sizeof(cliaddr);
    int n;

    len = clilen;
    n = recvfrom(modelsockasserverfd, record, sizeof(TickRecord), 0, &pcliaddr, &len);

    if (n == -1) n = 0;

    return n;

}

void disconnect()
{
    // Nothing to do
}


void setupControllerServer()
{
    controllersockfd = socket(AF_INET, SOCK_DGRAM, 0);
    fcntl(controllersockfd, F_SETFL, O_NONBLOCK);

    bzero(&controllerserveraddr, sizeof(controllerserveraddr));
    controllerserveraddr.sin_family = AF_INET;
    controllerserveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    controllerserveraddr.sin_port = htons(5000);

    bind(controllersockfd, (SA *) &controllerserveraddr, sizeof(controllerserveraddr));
}

void setupControllerClient()
{
    char ip[256];
    strcpy(ip, "192.168.1.186");
    //strcpy(ip, "127.0.0.1");
    int port = 5000;

    /* Clean up */
    bzero(&controllerserveraddr, sizeof(controllerserveraddr));

    /* Initialize the client to connect to the server on local port 4500 */
    controllerserveraddr.sin_family = AF_INET;
    controllerserveraddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &controllerserveraddr.sin_addr);

    /* Bring up the client socket */
    controllersockfd = socket(AF_INET, SOCK_DGRAM, 0);
}

void sendCommand(ControlStructure mesg)
{
    sendto(controllersockfd, &mesg, sizeof(mesg), 0, (SA *)&controllerserveraddr, sizeof(controllerserveraddr));
}

int receiveCommand(ControlStructure *mesg)
{
    socklen_t len;
    SA pcliaddr;
    struct sockaddr_in cliaddr;

    socklen_t clilen=sizeof(cliaddr);
    len = clilen;
    int n;

    n = recvfrom(controllersockfd, mesg, sizeof(ControlStructure), 0, &pcliaddr, &len);

    return n;
}
