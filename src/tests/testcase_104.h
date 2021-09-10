#ifndef TESTCASE_104_H
#define TESTCASE_104_H

#include "testcase.h"

class TestCase_104 : public TestCase
{
public:
    TestCase_104();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};
#endif // TESTCASE_104_H
