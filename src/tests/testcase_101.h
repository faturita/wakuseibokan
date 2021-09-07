#ifndef TESTCASE_101_H
#define TESTCASE_101_H

#include "testcase.h"


class TestCase_101 : public TestCase
{
public:
    TestCase_101();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_101_H
