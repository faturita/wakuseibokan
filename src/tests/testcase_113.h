#ifndef TESTCASE_113_H
#define TESTCASE_113_H


#include "testcase.h"

class TestCase_113 : public TestCase
{
private:
    size_t missile;
public:
    TestCase_113();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};


#endif // TESTCASE_113_H
