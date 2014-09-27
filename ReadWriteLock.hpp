/* definition for read write lock */

#ifndef READ_WRITE_LOCK_HPP
#define READ_WRITE_LOCK_HPP

#include "SemaphoreManager.hpp"

class ReadWriteLock
{
    public:
        ReadWriteLock();
        ReadWriteLock(SemaphorePair);
        ~ReadWriteLock();
        int acquireReadLock();
        int acquireWriteLock();
        int releaseReadLock();
        int releaseWriteLock();

    private:
        SemaphorePair mReadWriteLock;
};

#endif
