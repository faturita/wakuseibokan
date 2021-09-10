#ifndef TESTCASE_103_H
#define TESTCASE_103_H

#include "testcase.h"

class TestCase_103 : public TestCase
{
public:
    TestCase_103();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_103_H
