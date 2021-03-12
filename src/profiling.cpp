#include "profiling.h"

#include <iostream>
#include <cstdio>
#include <cstdarg>

#ifdef DEBUG
std::ostream &dout = std::cout;
#else
std::ofstream dev_null("/dev/null");
std::ostream &dout = dev_null;
#endif

bool CLog::m_bInitialised;
int  CLog::m_nLevel;

void CLog::Write(int nLevel, const char *szFormat, ...)
{
    CheckInit();
    if (nLevel >= m_nLevel)
    {
        va_list args;
        va_start(args, szFormat);
        vprintf(szFormat, args);
        va_end(args);
    }
}
void CLog::SetLevel(int nLevel)
{
    m_nLevel = nLevel;
    m_bInitialised = true;
}
void CLog::CheckInit()
{
    if (!m_bInitialised)
    {
        Init();
    }
}
void CLog::Init()
{
    int nDfltLevel(CLog::All);
    // Retrieve your level from an environment variable,
    // registry entry or wherecer
    SetLevel(nDfltLevel);
}

//int main()
//{
//    CLog::Write(CLog::Debug, "testing 1 2 3");
//    return 0;
//}
