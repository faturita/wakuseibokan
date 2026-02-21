#ifndef TESTCASE_126_H
#define TESTCASE_126_H

#include "testcase.h"

class TestCase_126 : public TestCase
{
public:
    TestCase_126();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_126_H
