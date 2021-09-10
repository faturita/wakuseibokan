#ifndef TESTCASE_102_H
#define TESTCASE_102_H

#include "testcase.h"

class TestCase_102 : public TestCase
{
public:
    TestCase_102();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_102_H
