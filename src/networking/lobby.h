#ifndef LOBBY_H
#define LOBBY_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/ioctl.h>

typedef struct sockaddr SA;

#include "../units/Vehicle.h"

void disconnect();
void init_lobby(char ip[256]);
void join_lobby();

void notify(unsigned long timerparam, size_t id, Vehicle *v);
int receive(TickRecord *record);

void setupControllerServer();
void setupControllerClient(char ip[256]);

void sendCommand(ControlStructure mesg);
int receiveCommand(ControlStructure *mesg);

int getlocalip(char ip[256]);


#endif // LOBBY_H
