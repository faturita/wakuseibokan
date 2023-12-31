#ifndef TESTCASE_118_H
#define TESTCASE_118_H

#include "testcase.h"

class TestCase_120 : public TestCase
{
public:
    TestCase_120();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_118_H
