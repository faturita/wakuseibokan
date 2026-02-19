#ifndef TESTCASE_123_H
#define TESTCASE_123_H

#include "testcase.h"

class TestCase_123 : public TestCase
{
public:
    TestCase_123();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_123_H
