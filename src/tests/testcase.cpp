//  testcase.cpp
//  wakuseibokan
//
//  Created by Rodrigo Ramele on 07/09/2021
//

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "testcase.h"

TestCase::TestCase()
{

}

TestCase::~TestCase()
{

}

int TestCase::number()
{
    return 0;
}

void TestCase::init()
{

}

int TestCase::check(unsigned long timertick)
{
    return 0;
}
std::string TestCase::title()
{
    return std::string("Generic Test Case");

}

bool TestCase::done()
{
    return isdone;
}
bool TestCase::passed()
{
    return false;
}
std::string TestCase::failedMessage()
{
    return message;
}
