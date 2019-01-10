#include "container.h"

#include <vector>
#include <mutex>
#include <unistd.h>

template<class T> container<T>::container()
{
    e_size = 0;

    for(int i=0;i<10000;i++)
        elem[i]= NULL;

}

template<class T> bool container<T>::isSafe()
{
    while (semaphore!=0) usleep(1);
    return true;
}

template<class T> void container<T>::lock()
{
    printf("<<");
    mlock.lock();
}

template<class T> void container<T>::unlock()
{
    mlock.unlock();
    printf(">>");
}

template<class T> void container<T>::lockme()
{
    semaphore=1;
}

template<class T> void container<T>::unlockme()
{
    semaphore=0;
    usleep(1);
}

template<class T> void container<T>::push_back(T value)
{
    size_t i=0;
    size_t count=0;
    for(i=0;i<10000;i++)
    {
        if (elem[i] == NULL)
            break;
    }

    elem[i] = value;
}


template<class T> T container<T>::operator[](size_t index)
{
    T t;
    //if (index>e_size)
    //    assert(!"This should not happen");
    if (elem[index] == NULL)
        assert(!"Pointer is null");
    //printf("Accessing %d\n", index);
    t = elem[index];
    return t;
}

template<class T> size_t container<T>::size()
{
    size_t i=0;
    size_t count=0;
    for(i=0;i<10000;i++)
    {
        if (elem[i] != NULL)
            count++;
    }
    return count;
}

template<class T> void container<T>::prune()
{
    for(int i=0;i<prunning.size();i++)
        delete prunning[i];
}

template<class T> bool container<T>::exists(size_t index)
{
    size_t i=index;
    while ( (i>=0 && i<10000) && elem[i] == NULL)
    {
        i++;
    }

    if (i>=10000)
    {
        return false;
    }
    else
    {
        return true;
    }
}

template<class T> size_t container<T>::first()
{
    size_t i=0;
    while ( (i>=0 && i<10000) && elem[i] == NULL) i++;
    return i;
}

template<class T> size_t container<T>::next(size_t index)
{
    size_t i=index+1;
    while ( (i>=0 && i<10000) && elem[i] == NULL) i++;

    return i;

}

template<class T> void container<T>::erase(size_t index)
{
    //prunning.push_back( *(elements.begin() + index) );
    //elements.erase(elements.begin() + index);

    T e = elem[index];

    if (elem[index] != NULL)
    {
        elem[index] = NULL;

        e_size--;

        delete e;
    }

}
