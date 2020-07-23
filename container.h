#ifndef CONTAINER_H
#define CONTAINER_H


// In Mac Sierra, if you change this 10000 value to something different, the bellow mutex does not work and generates a core dump :O
#define MAX 10000

#include <vector>
#include <mutex>
#include <unordered_map>

#include "ode/common.h"

#define synchronized(m) \
    for(std::unique_lock<std::recursive_mutex> lk(m); lk; lk.unlock())

template <class T> class container
{
protected:
    //std::vector<T> elements;
    std::mutex mlock;

    std::unordered_map<dGeomID, size_t> geomidmap;

    std::unordered_map<size_t, dGeomID> idmap;

    T elem[MAX];

    size_t e_size;

    int semaphore=0;

    std::vector<T> prunning;

    size_t push_back(T value);

public:
    /**
     * Use with the synchronized macro to restrict access to a block.
     * @brief m_mutex
     */
    std::recursive_mutex m_mutex;
    container();

    /**
     * Add a new value.
     *
     * @brief push_back
     * @param value
     * @return
     */
    size_t push_back(T value,dGeomID);

    /**
     * Verify whether is lock is released.
     *
     * @brief isSafe
     * @return
     */
    bool isSafe();

    /**
     * Index is an ID.
     *
     * @brief operator []
     * @param index
     * @return
     */
    T operator[](size_t index);

    T find(dGeomID element);

    /**
     * Returns the index for a given position (1-based).
     * @brief indexAt
     * @param position
     * @return
     */
    size_t indexAt(int position);

    /**
     * Returns the position (1-based) for a given index.
     *
     * @brief indexOf
     * @param index
     * @return
     */
    int    indexOf(size_t index);

    /**
     * Get the first element
     * @brief first
     * @return
     */
    size_t first();

    /**
     * Return the next index.
     * @brief next
     * @param index
     * @return
     */
    size_t next(size_t index);

    /**
     * Return true if there are more elements (to be retrieved by next).  Use me on for-loops.
     *
     * @brief exists
     * @param index
     * @return
     */
    bool hasMore(size_t index);

    /**
     * Size of the container.
     *
     * @brief size
     * @return
     */
    size_t size();

    /**
     * Erase thread-safely the element T at index.
     * @brief erase
     * @param index
     */
    void erase(size_t index);
    void erase(dGeomID geom);

    void prune();

    void lockme();

    void unlockme();

    void lock();

    void unlock();

    /**
     * Return true is index is valid.
     *
     * @brief isValid
     * @param index
     * @return
     */
    bool isValid(size_t index);






};

// This is a nasty solution to allow the linking of the template... Have a better one? Do it.
#include "container.cpp"

#endif // CONTAINER_H
