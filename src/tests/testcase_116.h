#ifndef TESTCASE_116_H
#define TESTCASE_116_H


#include "testcase.h"

class TestCase_116 : public TestCase
{
public:
    TestCase_116();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_116_H
