#ifndef TESTCASE_H
#define TESTCASE_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

class TestCase
{
protected:
    bool isdone=false;
    bool haspassed=false;
    std::string message;
public:
    TestCase();
    virtual ~TestCase();

    // This method is called when the test is initialized.  It should create islands and all the other entities.
    virtual void init();

    // This method is called at each simulation step.  The method should check the completion of the code and returns a return value (0 error).
    virtual int check(unsigned long timertick);

    // Title and number of the testcase.
    virtual std::string title();
    virtual int number();

    virtual bool done();
    virtual bool passed();
    virtual std::string failedMessage();
};

// This method should be implemented by the test case.  This allows the method to create the particular instance.
// The idea is to go through the code and use the compilation scheme to create all the available test cases as long as there are classes that implement
// this.
TestCase *pickTestCase(int testcase);

#endif // TESTCASE_H
