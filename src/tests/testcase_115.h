#ifndef TESTCASE_115_H
#define TESTCASE_115_H


#include "testcase.h"

class TestCase_115 : public TestCase
{
public:
    TestCase_115();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};


#endif // TESTCASE_115_H
