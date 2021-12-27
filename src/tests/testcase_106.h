#ifndef TESTCASE_106_H
#define TESTCASE_106_H

#include "testcase.h"

class TestCase_106 : public TestCase
{
public:
    TestCase_106();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};


#endif // TESTCASE_106_H
