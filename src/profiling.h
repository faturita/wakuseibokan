#ifndef PROFILING_H
#define PROFILING_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Comment the following define to allow quick suppression of all debug printfs or couts.
#define DEBUG

extern std::ostream &dout;

class CLog
{
public:
    enum { All=0, Debug, Info, Warning, Error, Fatal, None };
    static void Write(int nLevel, const char *szFormat, ...);
    static void SetLevel(int nLevel);

protected:
    static void CheckInit();
    static void Init();

private:
    CLog();
    static bool m_bInitialised;
    static int  m_nLevel;
};


#endif // PROFILING_H
