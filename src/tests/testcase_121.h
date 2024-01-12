#ifndef TESTCASE_121_H
#define TESTCASE_121_H

#include "testcase.h"

class TestCase_121 : public TestCase
{
public:
    TestCase_121();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};



#endif // TESTCASE_121_H
