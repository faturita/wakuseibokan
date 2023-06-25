#ifndef TESTCASE_109_H
#define TESTCASE_109_H

#include "testcase.h"

class TestCase_110 : public TestCase
{
public:
    TestCase_110();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};



#endif // TESTCASE_110_H
