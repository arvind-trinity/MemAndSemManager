
#include "Semaphore.hpp"
#include "SemaphoreManager.hpp"
#include <sys/sem.h>
#include <sys/errno.h>

#if 0
union semun
{
    int val;
    struct semid_ds *buf;
    ushort *array;
    struct seminfo *__buf;
    void *__pad;
};
#endif

Semaphore::Semaphore(int id,int index):mId(id),mIndex(index)
{}

Semaphore::Semaphore():mId(0),mIndex(0)
{
#if 0
    Semaphore tempSemaphore(SemaphoreManager::create()->getSemaphore());
    mId = tempSemaphore.getId();
    mIndex = tempSemaphore.getIndex();
#endif
}

int Semaphore::getCount()
{
    return semctl(mId,mIndex,GETVAL);
}

int Semaphore::setCount(int count)
{
    union semun semopts;

    semopts.val = count;
    if(-1 == semctl(mId,mIndex,SETVAL,semopts))
    {
        return 0;
    }
    return 1;
}

int Semaphore::increaseCount()
{
    struct sembuf options;

    options.sem_num = mIndex;
    options.sem_op = 1;
    options.sem_flg = SEM_UNDO | IPC_NOWAIT;

    if(-1 == semop(mId,&options,1))
    {
        return 0;
    }
    return 1;
}

int Semaphore::decreaseCount()
{
    struct sembuf options[1];

    options[0].sem_num = mIndex;
    options[0].sem_op = -1;
    options[0].sem_flg = SEM_UNDO | IPC_NOWAIT;

    if(-1 == semop(mId,options,1))
    {
        return 0;
    }

    return 1;
}
