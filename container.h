#ifndef CONTAINER_H
#define CONTAINER_H

#include <vector>
#include <mutex>

template <class T> class container
{
protected:
    std::vector<T> elements;
    std::mutex lock;

    std::vector<T> prunning;
public:
    container();

    void push_back(T value);

    T operator[](int index);

    size_t size();

    void erase(int index);

    void prune();

};

// This is a nasty solution to allow the linking of the template... Have a better one? Do it.
#include "container.cpp";

#endif // CONTAINER_H
