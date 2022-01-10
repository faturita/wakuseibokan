#ifndef TESTCASE_108_H
#define TESTCASE_108_H

#include "testcase.h"

class TestCase_108 : public TestCase
{
public:
    TestCase_108();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};


#endif // TESTCASE_108_H
