#ifndef TESTCASE_129_H
#define TESTCASE_129_H

#include "testcase.h"

class TestCase_129 : public TestCase
{
public:
    TestCase_129();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_129_H
