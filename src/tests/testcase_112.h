#ifndef TESTCASE_112_H
#define TESTCASE_112_H

#include "testcase.h"

class TestCase_112 : public TestCase
{
private:
    unsigned long endtimer;
    int whowon;
public:
    TestCase_112();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};


#endif // TESTCASE_112_H
