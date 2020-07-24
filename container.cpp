#include "container.h"

#include <vector>
#include <mutex>
#include <unistd.h>
#include <unordered_map>

template<class T> container<T>::container()
{
    e_size = 0;

    for(int i=0;i<MAX;i++)
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


template<class T> size_t container<T>::push_back(T value, dGeomID geom)
{
    size_t i = push_back(value);
    geomidmap[geom] = i;
    idmap[i] = geom;
    return i;
}

template<class T> size_t container<T>::push_back(T value)
{
    size_t i=0;
    size_t count=0;
    for(i=0;i<MAX;i++)
    {
        if (elem[i] == NULL)
            break;
    }

    assert( i < MAX || !"The container is full and cannot hold more values.");

    elem[i] = value;


    return i;
}


template<class T> T container<T>::operator[](size_t index)
{
    T t;
    if (index>MAX)
        assert(!"This should not happen");
    if (elem[index] == NULL)
        assert(!"Pointer is null");
    //printf("Accessing %d\n", index);
    t = elem[index];
    return t;
}

template<class T> T container<T>::find(dGeomID geom)
{
    size_t i = geomidmap[geom];

    if (isValid(i))
    {
        return operator[](i);
    }
    else
    {
        return NULL;
    }
}

template <class T> size_t container<T>::indexAt(int position)
{
    size_t i=0;
    int count=0;
    for(i=0;i<MAX;i++)
    {
        if (elem[i] != NULL)
            count++;

        if (count == position)
            break;
    }

    return i;
}

template <class T> int container<T>::indexOf(size_t element)
{
    size_t i=0;
    int count=0;
    for(i=0;i<MAX;i++)
    {
        if (elem[i] != NULL)
            count++;

        if (i == element)
            return count;
    }

    return 0;
}

template<class T> size_t container<T>::size()
{
    size_t i=0;
    size_t count=0;
    for(i=0;i<MAX;i++)
    {
        if (elem[i] != NULL)
            count++;
    }
    return count;
}

template<class T> void container<T>::prune()
{
    assert(!"Deprecated!");

    for(int i=0;i<prunning.size();i++)
        delete prunning[i];
}

template<class T> bool container<T>::isValid(size_t index)
{
    return (elem[index] != NULL);
}


template<class T> bool container<T>::hasMore(size_t index)
{
    size_t i=index;
    while ( (i>=0 && i<MAX) && elem[i] == NULL)
    {
        i++;
    }

    if (i>=MAX)
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
    while ( (i>=0 && i<MAX) && elem[i] == NULL) i++;
    return i;
}

template<class T> size_t container<T>::next(size_t index)
{
    size_t i=index+1;
    while ( (i>=0 && i<MAX) && elem[i] == NULL) i++;

    return i;

}

template<class T> void container<T>::erase(dGeomID geom)
{
    size_t index = geomidmap[geom];

    T e = elem[index];

    if (elem[index] != NULL)
    {
        geomidmap.erase(geom);

        elem[index] = NULL;

        e_size--;

        delete e;
    }
}

