#include "container.h"

#include <vector>
#include <mutex>
#include <unistd.h>
#include <unordered_map>

template<class T> container<T>::container()
{
    e_size = 0;

    for(int i=MIN;i<MAX;i++)
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
#ifndef _WIN32
    mlock.lock();
#endif
}

template<class T> void container<T>::unlock()
{
#ifndef _WIN32
    mlock.unlock();
#endif
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
    return push_back(MIN,value,geom);
}

template<class T> size_t container<T>::push_at_the_back(T value, dGeomID geom)
{
    return push_back(e_size, value, geom);
}

template<class T> size_t container<T>::push_back(size_t initialregion, T value, dGeomID geom)
{
    size_t i = push_back(initialregion, value);
    geomidmap[geom] = i;
    return i;
}

template<class T> size_t container<T>::push_back(T value)
{
    return push_back(MIN, value);
}

template<class T> size_t container<T>::push_back(size_t initialregion, T value)
{
    size_t i;
    for(i=initialregion;i<MAX;i++)
    {
        if (elem[i] == NULL)
            break;
    }

    assert( i < MAX || !"The container is full and cannot hold more values.");

    elem[i] = value;

    e_size++;

    return i;
}


template<class T> T container<T>::operator[](size_t index)
{
    T t;
    if (index<MIN || index>MAX)
        assert(!"This should not happen");
    if (elem[index] == NULL)
        assert(!"Pointer is null");
    //printf("Accessing %d\n", index);
    t = elem[index];
    return t;
}

template<class T> T container<T>::find(dGeomID geom)
{
    std::unordered_map<dGeomID, size_t>::const_iterator got = geomidmap.find (geom);

    if ( got == geomidmap.end() )
    {
        return NULL;
    }

    //std::cout << got->first << " is " << got->second;

    // @NOTE: If the key doesn't exists it gets created !!!  If you have to check before if it is present or not.
    size_t i = got->second; // geomidmap[geom];

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
    size_t i=MIN;
    int count=0;
    for(i=MIN;i<MAX;i++)
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
    size_t i=MIN;
    int count=0;
    for(i=MIN;i<MAX;i++)
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
    size_t i=MIN;
    size_t count=0;
    for(i=MIN;i<MAX;i++)
    {
        if (elem[i] != NULL)
            count++;
    }
    return count;
}

template<class T> void container<T>::prune()
{
    assert(!"Deprecated!");

    for(int i=MIN;i<prunning.size();i++)
        delete prunning[i];
}

template<class T> bool container<T>::isValid(size_t index)
{
    // @NOTE: Allow zero because I want to return FALSE for it.
    assert( index>=0 && index <MAX || !"Accessing an entity which is out of the valid range.");

    return (elem[index] != NULL);
}


template<class T> bool container<T>::hasMore(size_t index)
{
    size_t i=index;
    while ( (i>=MIN && i<MAX) && elem[i] == NULL)
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
    size_t i=MIN;
    while ( (i>=MIN && i<MAX) && elem[i] == NULL) i++;
    return i;
}

template<class T> size_t container<T>::next(size_t index)
{
    size_t i=index+1;
    while ( (i>=MIN && i<MAX) && elem[i] == NULL) i++;

    return i;

}

// The container is the ONLY component responsible of deleting all the entities.
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

