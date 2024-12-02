#ifndef TESTCASE_122_H
#define TESTCASE_122_H

#include "testcase.h"

class TestCase_122 : public TestCase
{
public:
    TestCase_122();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_122_H
