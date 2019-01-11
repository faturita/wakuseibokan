#ifndef CONTAINER_H
#define CONTAINER_H

#define MAX 10000

#include <vector>
#include <mutex>

#define synchronized(m) \
    for(std::unique_lock<std::recursive_mutex> lk(m); lk; lk.unlock())

template <class T> class container
{
protected:
    //std::vector<T> elements;
    std::mutex mlock;

    T elem[MAX];

    size_t e_size;

    int semaphore=0;

    std::vector<T> prunning;
public:
    std::recursive_mutex m_mutex;
    container();

    size_t push_back(T value);

    bool isSafe();

    T operator[](size_t index);

    size_t first();
    size_t next(size_t index);
    bool exists(size_t index);

    size_t size();

    void erase(size_t index);

    void prune();

    void lockme();

    void unlockme();

    void lock();

    void unlock();

};

// This is a nasty solution to allow the linking of the template... Have a better one? Do it.
#include "container.cpp"

#endif // CONTAINER_H
