#ifndef TESTCASE_119_H
#define TESTCASE_119_H

#include "testcase.h"

class TestCase_119 : public TestCase
{
public:
    TestCase_119();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};

#endif // TESTCASE_119_H
