/* member functions of the class Read write lock */

#include "Lock.hpp"

Lock::Lock():mFlag(false)
{
    mLock = SemaphoreManager::create()->getSemaphore();
}

Lock::Lock(Semaphore sem):mLock(sem),mFlag(true){}

Lock::~Lock()
{
    if(mFlag)
    {
        semctl(mLock.getId(),0, IPC_RMID);
    }
    else
    {
        SemaphoreManager::create()->freeSemaphore(mLock);
    }
}

int Lock::acquireLock()
{
    struct sembuf options[2];

    //wait for unlock
	options[0].sem_num = mLock.getIndex();
	options[0].sem_op = 0;
	options[0].sem_flg = SEM_UNDO;

    //set lock
    options[1].sem_num = mLock.getIndex();
    options[1].sem_op = 1;
    options[1].sem_flg = SEM_UNDO | IPC_NOWAIT;

    if(-1 == semop(mLock.getId(),options,1))
    {
        return 0;
    }
    return 1;
}

int Lock::releaseLock()
{
    struct sembuf options;

    //unlock
    options.sem_num = mLock.getIndex();
    options.sem_op = -1;
    options.sem_flg = SEM_UNDO | IPC_NOWAIT;

    if(-1 == semop(mLock.getId(),&options,1))
    {
        return 0;
    }
    return 1;
}
