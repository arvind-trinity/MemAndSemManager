/* 
 * definition for lock that operates like a mutex 
 * uses one semaphore. 0 is unlock and 1 is lock
 *
 * when locked waits for the lock to be cleared, this blocks.
 * locks if a lock is not locked and returns
 */

#ifndef LOCK_HPP
#define LOCK_HPP

#include "Semaphore.hpp"

class Lock
{
    public:
        Lock();
        Lock(Semaphore);
        ~Lock();
        int acquireLock();
        int releaseLock();

    private:
        Semaphore mLock;
        bool mFlag;
};

#endif
