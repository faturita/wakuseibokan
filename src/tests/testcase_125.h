#ifndef TESTCASE_125_H
#define TESTCASE_125_H

#include "testcase.h"

class TestCase_125 : public TestCase
{
public:
    TestCase_125();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_125_H
