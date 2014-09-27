/* definition for lock that operates like a mutex */

#ifndef LOCK_HPP
#define LOCK_HPP

#include "Semaphore.hpp"
#include "SemaphoreManager.hpp"

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
