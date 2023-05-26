#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ledger.h"

#include "telemetry.h"


struct LobbyConnection
{
    int sockfd;
    struct sockaddr_in servaddr;
};

std::vector<LobbyConnection> lobby;

LobbyConnection newplayer(char ip[], int port)
{
    LobbyConnection con;

    /* Clean up */
    bzero(&con.servaddr, sizeof(con.servaddr));

    /* Initialize the client to connect to the server on local port 4500 */
    con.servaddr.sin_family = AF_INET;
    con.servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &con.servaddr.sin_addr);

    /* Bring up the client socket */
    con.sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    return con;
}

void init_lobby()
{
    // @FIXME: Read the parameters from a configuration file or from some structure with the lobby information
    // (who has joined the game)
    //lobby.push_back(newplayer("127.0.0.1",4500));
    lobby.push_back(newplayer("192.168.1.195",4500));
}


struct sockaddr_in servaddr, cliaddr;
int sockfd;

void join_lobby()
{
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    //fcntl(sockfd, F_SETFL, O_NONBLOCK);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(4500);

    bind(sockfd, (SA *) &servaddr, sizeof(servaddr));
}


void record(unsigned long timerparam, size_t id, Vehicle* v)
{
    TickRecord tickrecord;

    tickrecord = v->serialize();

    tickrecord.timerparam = timerparam;
    tickrecord.id = id;

    fwrite(&tickrecord, sizeof(TickRecord),1,ledger);
}

void notify(unsigned long timerparam, size_t id, Vehicle *v)
{
    TickRecord tickrecord;

    tickrecord = v->serialize();

    tickrecord.timerparam = timerparam;
    tickrecord.id = id;

    for (size_t i=0; i<lobby.size(); i++) {
        // @NOTE: The socket must be already connected at this point.
        sendto(lobby[i].sockfd, &tickrecord, sizeof(tickrecord), 0, (SA *)&lobby[i].servaddr, sizeof(lobby[i].servaddr));
    }
}

int receive(TickRecord *record)
{
    socklen_t len;
    SA pcliaddr;

    socklen_t clilen=sizeof(cliaddr);
    int n;

    len = clilen;
    n = recvfrom(sockfd, record, sizeof(TickRecord), 0, &pcliaddr, &len);

    if (n == -1) n = 0;

    return n;

}

void disconnect()
{
    // Nothing to do
}



