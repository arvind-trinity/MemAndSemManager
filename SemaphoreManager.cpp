/* definition of semaphore manager */

#include "SemaphoreManager.hpp"
#include "String.hpp"
#include <fstream>
#include <stdlib.h>
#include "Lock.hpp"
#include <sys/errno.h>

Lock *gLock = NULL;
SemaphoreManager* SemaphoreManager::mpInstance = NULL;

SemaphoreManager* SemaphoreManager::create()
{
    if(!mpInstance)
    {
        mpInstance = new SemaphoreManager();
    }

    return mpInstance;
}

void SemaphoreManager::destroy()
{
    delete mpInstance;
    mpInstance = NULL;
}

/*
 * see if there is a any semaphore free in mFreeSemList
 * if no then see in mCurrentSemArray if no allocate a
 * semaphore array and return a semaphore
 */
Semaphore SemaphoreManager::getSemaphore()
{
    bool done = false;
    Semaphore sem;

    gLock->acquireLock();
    if(mFreeSemList.size() > 0) //check if we have free sem
    {
        for(SemaphoreArrayItr itr = mFreeSemList.begin();
                itr != mFreeSemList.end();itr++)
        {
            if(itr->second.size()>0)
            {
                sem.mId = itr->first;
                sem.mIndex = itr->second.front();
                itr->second.pop_front();
                done = true;
                break;
            }
        }
    }

    if(!done) //not yet allocated
    {
        if(mCurrentSemArray.mIndex >= mMaxSemInArray)
            //check if current array has anything left
        {
            int semId = 0;
            if((semId = getSemaphoreArray()) == 0)
            {
                /* error */
                gLock->releaseLock();
                return sem;
            }

            /* create a semophore array and push into free list */
            std::list<int> indexes;
            mFreeSemList[semId] = indexes;

            /* set the current array */
            mCurrentSemArray.mId = semId;
            mCurrentSemArray.mIndex = 0;
        }
        sem.mId = mCurrentSemArray.mId;
        sem.mIndex = mCurrentSemArray.mIndex++;
    }

    gLock->releaseLock();
    return sem;
}

void SemaphoreManager::freeSemaphore(Semaphore sem)
{
    gLock->acquireLock();
    std::list<int> indexes = mFreeSemList[sem.mId];
    indexes.push_back(sem.mIndex);
    mFreeSemList[sem.mId] = indexes;
    gLock->releaseLock();
}

SemaphorePair SemaphoreManager::getSemaphorePair()
{
    SemaphorePair semPair = {0};
    bool done = false;

    gLock->acquireLock();
    if(mFreeSemList.size() > 0) //check if we have free sem
    {
        for(SemaphoreArrayItr itr = mFreeSemList.begin();
                itr != mFreeSemList.end();itr++)
        {
            if(itr->second.size()>1)
            {
                semPair.id = itr->first;
                semPair.firstIndex = itr->second.front();
                itr->second.pop_front();
                semPair.secondIndex = itr->second.front();
                itr->second.pop_front();
                done = true;
                break;
            }
        }
    }

    if(!done) //not yet allocated
    {
        if(mCurrentSemArray.mIndex >= mMaxSemInArray)
            //check if current array has anything left
        {
            int semId = 0;
            if((semId = getSemaphoreArray()) == 0)
            {
                /* error */
                gLock->releaseLock();
                return semPair;
            }

            /* create a semophore array and push into free list */
            std::list<int> indexes;
            mFreeSemList[semId] = indexes;

            /* set the current array */
            mCurrentSemArray.mId = semId;
            mCurrentSemArray.mIndex = 0;
        }
        semPair.id = mCurrentSemArray.mId;
        semPair.firstIndex = mCurrentSemArray.mIndex++;
        semPair.secondIndex = mCurrentSemArray.mIndex++;
    }

    gLock->releaseLock();
    return semPair;
}

void SemaphoreManager::freeSemaphorePair(SemaphorePair semPair)
{
    gLock->acquireLock();
    std::list<int> indexes = mFreeSemList[semPair.id];
    indexes.push_back(semPair.firstIndex);
    indexes.push_back(semPair.secondIndex);
    mFreeSemList[semPair.id] = indexes;
    gLock->releaseLock();
}

int SemaphoreManager::initializeSemaphoreSize()
{
    int ret = 0;

    String details;
    std::ifstream file(SEM_DETAILS_LOCATION);
    if(file.is_open()){
        getline(file, details);
        mMaxSemInArray = atoi(details.substr(0, details.find(' ')).c_str());
        mMaxSemArray = atoi(details.substr(details.find_last_of(" ") + 1,
                    details.size() - 1).c_str());
        ret = 1;
    }

    return ret;
}

/* create and initialize */
int SemaphoreManager::getSemaphoreArray()
{
    int semId = 0;

    /* create semaphore array */
    if((semId = semget(IPC_PRIVATE, mMaxSemInArray, 0666 | IPC_CREAT)) == -1)
    {
        return 0;
    }

    //TODO: fix this
#if 0 
    /* initialize semaphore array */
    if(semctl(semId, 0, SETALL, arg) == -1)
    {
        semctl(semId,0, IPC_RMID);
        return 0;
    }
#endif

    return semId;
}

void SemaphoreManager::freeSemaphoreArray(int semId)
{
    semctl(semId,0, IPC_RMID);
}

int SemaphoreManager::initializeSemaphoreList()
{
    int ret = 0,semId=0;

    if((semId = getSemaphoreArray()) == 0)
    {
        return 0;
    }

    /* initialize member data */
    mCurrentSemArray.mId = semId;
    mCurrentSemArray.mIndex = 0;

    /* 
     * initialize the global lock 
     * give the first semaphore for lock
     */
    Semaphore sem;
    sem.mId = mCurrentSemArray.mId;
    sem.mIndex = mCurrentSemArray.mIndex++;
    gLock = new Lock(sem);

    return ret;
}

SemaphoreManager::SemaphoreManager():mMaxSemInArray(0),
    mMaxSemArray(0)
{

    /* initialize lock */
    //gLock = new Lock(Semaphore(
    //semget(IPC_PRIVATE,1,0666|IPC_CREAT),0));

    /* clear the lists */
    mFreeSemList.clear();
    mCurrentSemArray.mId = 0;
    mCurrentSemArray.mIndex = 0;

    /* initialize */
    initializeSemaphoreSize();
    initializeSemaphoreList();
}

SemaphoreManager::~SemaphoreManager()
{
    /* free all the semaphores */
    for(SemaphoreArrayItr itr = mFreeSemList.begin();
            itr != mFreeSemList.end();itr++)
    {
        freeSemaphoreArray(itr->first);
    }

    /* clear the lists */
    mFreeSemList.clear();
    delete gLock;
}

