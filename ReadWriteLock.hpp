/* 
 * definition for read write lock
 * uses a semaphore pair for the implementation, requirement is both 
 * semaphores must from a single array.
 * one semaphore is used as read lock and the other as write lock.
 * read lock is incremental value, has the count of number of 
 * read actions that are currently active on the resource protected by the 
 * lock. Write lock is either 1 (locked) or 0 (unlocked).
 *
 * To acquire a read lock, write lock should be zero. To acquire a write lock,
 * both read and write lock must be 0.
 */

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
