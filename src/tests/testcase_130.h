#ifndef TESTCASE_130_H
#define TESTCASE_130_H

#include "testcase.h"

class TestCase_130 : public TestCase
{
public:
    TestCase_130();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_130_H
