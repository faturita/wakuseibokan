#ifndef TESTCASE_105_H
#define TESTCASE_105_H

#include "testcase.h"

class TestCase_105 : public TestCase
{
public:
    TestCase_105();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_105_H
