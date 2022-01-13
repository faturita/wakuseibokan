#ifndef TESTCASE_109_H
#define TESTCASE_109_H

#include "testcase.h"

class TestCase_109 : public TestCase
{
public:
    TestCase_109();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};



#endif // TESTCASE_109_H
