#ifndef TESTCASE_111_H
#define TESTCASE_111_H

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

#include "testcase.h"

class TestCase_111 : public TestCase
{
private:
    struct sockaddr_in servaddr, cliaddr;
    int sockfd;
    unsigned long endtimer;
    int whowon;
public:
    TestCase_111();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};



#endif // TESTCASE_111_H
