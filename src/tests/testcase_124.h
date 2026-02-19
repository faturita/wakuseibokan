#ifndef TESTCASE_124_H
#define TESTCASE_124_H

#include "testcase.h"

class TestCase_124 : public TestCase
{
public:
    TestCase_124();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_124_H
