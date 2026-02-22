#ifndef TESTCASE_127_H
#define TESTCASE_127_H

#include "testcase.h"

class TestCase_127 : public TestCase
{
public:
    TestCase_127();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_127_H
