#ifndef TELEMETRY_H
#define TELEMETRY_H

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

char *Sock_ntop (const struct sockaddr *sa, socklen_t salen) ;

void telemetryme(int number, int health, int power, float bearing, float *dBodyPosition, float *dBodyRotation);

void inittelemetry();


#endif // TELEMETRY_H
