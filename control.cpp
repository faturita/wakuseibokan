#include <string.h>
#include "control.h"

void  getIntegrativeTerm(std::vector<std::vector<float> > accerr,float integrative[3],float e[3])
{
    if (accerr.size()>200)
        accerr.erase(accerr.begin());

    std::vector<float> err;

    for(int i=0;i<3;i++) err.push_back(e[i]);

    accerr.push_back(err);

    memset(integrative,0,sizeof(float[3]));

    for(size_t j=0;j<accerr.size();j++)
    {
        for(int i=0;i<3;i++)
            integrative[i] += accerr[j][i];
    }
}

