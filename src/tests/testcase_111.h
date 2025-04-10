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

#define DEFAULT_MATCH_DURATION 5000

struct socketaddr
{
    struct sockaddr_in servaddr, cliaddr;
    int sockfd;
};

class TestCase_111 : public TestCase
{
private:
    unsigned long endtimer = DEFAULT_MATCH_DURATION;
    int whowon;
    int iendpoints;

    std::vector<float> healthes;
    std::vector<float> powers;
    std::vector<float> distances;
    std::vector<struct socketaddr> sockadders;


    void checkBeforeDone(unsigned long timertick);

public:
    TestCase_111();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
    size_t addTank(BoxIsland *island, int faction, int walusnumber, GLuint textureId);
    void reset(BoxIsland *island);
    void cleanall();
};



#endif // TESTCASE_111_H
