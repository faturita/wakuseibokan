#ifndef TESTCASE_117_H
#define TESTCASE_117_H



#include "testcase.h"

class TestCase_117 : public TestCase
{
public:
    TestCase_117();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};


#endif // TESTCASE_117_H
