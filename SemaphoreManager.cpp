/* 
 * implementation of Semaphore Manager
 */

#include "SemaphoreManager.hpp"
#include <fstream>
#include <stdlib.h>
#include "Lock.hpp"
#include <sys/errno.h>

SemaphoreManager* SemaphoreManager::mpInstance = NULL;

/* static create function for singleton implementation */
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

    mpLock->acquireLock();
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
                mpLock->releaseLock();
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

    mpLock->releaseLock();
    return sem;
}

void SemaphoreManager::freeSemaphore(Semaphore sem)
{
    mpLock->acquireLock();
    std::list<int> indexes = mFreeSemList[sem.mId];
    indexes.push_back(sem.mIndex);
    mFreeSemList[sem.mId] = indexes;
    mpLock->releaseLock();
}

SemaphorePair SemaphoreManager::getSemaphorePair()
{
    SemaphorePair semPair = {0};
    bool done = false;

    mpLock->acquireLock();
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
                mpLock->releaseLock();
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

    mpLock->releaseLock();
    return semPair;
}

void SemaphoreManager::freeSemaphorePair(SemaphorePair semPair)
{
    mpLock->acquireLock();
    std::list<int> indexes = mFreeSemList[semPair.id];
    indexes.push_back(semPair.firstIndex);
    indexes.push_back(semPair.secondIndex);
    mFreeSemList[semPair.id] = indexes;
    mpLock->releaseLock();
}

/* system wide settings are loaded from the proc file */
int SemaphoreManager::initializeSemaphoreSize()
{
    int ret = 0;

    std::string details;
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

/* creates a semaphore array and returns the sem id */
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

/* frees a semaphore array */
void SemaphoreManager::freeSemaphoreArray(int semId)
{
    semctl(semId,0, IPC_RMID);
}

/* 
 * function is called once when the semaphore manager is created
 * creates an semaphore array and sets the index in mCurrentSemArray
 * creates a lock for sync only after the semaphore manager is initialized
 * becasue Lock uses semaphore manager to manage sem resources.
 */
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

    /* initialize mFreeList */
    std::list<int> list;
    mFreeSemList[semId] = list;

    /* 
     * initialize the lock 
     * give the first semaphore for lock
     */
    Semaphore sem;
    sem.mId = mCurrentSemArray.mId;
    sem.mIndex = mCurrentSemArray.mIndex++;
    mpLock = new Lock(sem);

    return ret;
}

SemaphoreManager::SemaphoreManager():mMaxSemInArray(0),
    mMaxSemArray(0)
{

    /* initialize lock */
    //mpLock = new Lock(Semaphore(
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
    delete mpLock;
}

