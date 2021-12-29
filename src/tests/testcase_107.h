#ifndef TESTCASE_107_H
#define TESTCASE_107_H

#include "testcase.h"

class TestCase_107 : public TestCase
{
public:
    TestCase_107();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};


#endif // TESTCASE_107_H
