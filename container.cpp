#include "container.h"

#include <vector>
#include <mutex>

template<class T> container<T>::container()
{

}

template<class T> void container<T>::push_back(T value)
{
    lock.lock();
    elements.push_back(value);
    lock.unlock();
}

template<class T> T container<T>::operator[](int index)
{
    T t;
    lock.lock();
    if (index>elements.size())
        assert(!"This should not happen");
    t = elements[index];
    lock.unlock();
    return t;
}

template<class T> void container<T>::prune()
{
    for(int i=0;i<prunning.size();i++)
        delete prunning[i];
}

template<class T> size_t container<T>::size()
{
    lock.lock();
    size_t s = elements.size();

    prune();
    lock.unlock();
    return s;
}

template<class T> void container<T>::erase(int index)
{
    lock.lock();
    prunning.push_back( *(elements.begin() + index) );
    elements.erase(elements.begin() + index);
    lock.unlock();
}
