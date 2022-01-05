#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../units/Vehicle.h"
#include "../usercontrols.h"

#include "testcase_101.h"

extern unsigned long timer;

extern  Controller controller;

extern int testing;

TestCase_101::TestCase_101()
{

}

void TestCase_101::init()
{

}

int TestCase_101::check(unsigned long timertick)
{

    if (timertick > 1000)
    {
        isdone = true;
        haspassed = false;
        message = std::string("The timeout has occurred and nothing happened.");
    }


    return 0;
}

int TestCase_101::number()
{
    return 101;

}

std::string TestCase_101::title()
{
    return std::string("Basic Test Template.");
}


bool TestCase_101::done()
{
    return isdone;
}
bool TestCase_101::passed()
{
    return haspassed;
}
std::string TestCase_101::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_101();
}

