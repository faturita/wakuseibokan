#ifndef TESTCASE_114_H
#define TESTCASE_114_H


#include "testcase.h"

class TestCase_114 : public TestCase
{
private:
    size_t missile;
public:
    TestCase_114();
    void init();
    int check(unsigned long timertick);
    std::string title();
    int number();
    bool done();
    bool passed();
    std::string failedMessage();
};



#endif // TESTCASE_114_H
