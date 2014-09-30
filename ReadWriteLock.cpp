/* implementation of read wrtie lock */

#include "ReadWriteLock.hpp"
#include <sys/errno.h>

ReadWriteLock::ReadWriteLock()
{
    mReadWriteLock = 
        SemaphoreManager::create()->getSemaphorePair();
}

ReadWriteLock::ReadWriteLock(SemaphorePair semPair)
    :mReadWriteLock(semPair){}

ReadWriteLock::~ReadWriteLock()
{
    SemaphoreManager::create()->
        freeSemaphorePair(mReadWriteLock);
}

/* 
 * waits for write lock to be released, 
 * then increments the read count by 1 
 */
int ReadWriteLock::acquireReadLock()
{
    struct sembuf options[2];

    //check write lock
    options[0].sem_num = mReadWriteLock.secondIndex;
    options[0].sem_op = 0;
    options[0].sem_flg = SEM_UNDO;

    //set read lock
    options[1].sem_num = mReadWriteLock.firstIndex;
    options[1].sem_op = 1;
    options[1].sem_flg = SEM_UNDO | IPC_NOWAIT;

    if(-1 == semop(mReadWriteLock.id,options,2))
    {
        return 0;
    }
    return 1;
}

/* 
 * wait for write lock to be unlocked, then locks the write lock
 * waits till the read lock is zero before returning
 */
int ReadWriteLock::acquireWriteLock()
{
    struct sembuf options[3];

    //wait for write lock to become zero
    options[0].sem_num = mReadWriteLock.secondIndex;
    options[0].sem_op = 0;
    options[0].sem_flg = SEM_UNDO;

    //set write lock
    options[1].sem_num = mReadWriteLock.secondIndex;
    options[1].sem_op = 1;
    options[1].sem_flg = SEM_UNDO | IPC_NOWAIT;

    //wait for read lock to become zero
    options[2].sem_num = mReadWriteLock.firstIndex;
    options[2].sem_op = 0;
    options[2].sem_flg = SEM_UNDO;

    if(semop(mReadWriteLock.id,options,3) == -1)
    {
        return 0;
    }
    return 1;
}

/* decrements the read lock */
int ReadWriteLock::releaseReadLock()
{
    struct sembuf options[1];

    options[0].sem_num = mReadWriteLock.firstIndex;
    options[0].sem_op = -1;
    options[0].sem_flg = SEM_UNDO | IPC_NOWAIT;

    if(-1 == semop(mReadWriteLock.id,options,1))
    {
        return 0;
    }
    return 1;
}

/* sets the read lock to zero */
int ReadWriteLock::releaseWriteLock()
{
    struct sembuf options[1];

    options[0].sem_num = mReadWriteLock.secondIndex;
    options[0].sem_op = -1;
    options[0].sem_flg = SEM_UNDO | IPC_NOWAIT;

    if(-1 == semop(mReadWriteLock.id,options,1))
    {
        return 0;
    }
    return 1;
}

