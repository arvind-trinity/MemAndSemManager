/* implementation of memory manager */

#include <syslog.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include "MemoryManager.hpp"

/* global functions to allocate and free memory */
void* memAlloc(size_t size)
{
    return MemoryManager::get()->memAlloc(size);
}

void memFree(void *mem)
{
    MemoryManager::get()->memFree(mem);
}

MemoryManager* MemoryManager::instance = NULL;

/* 
 * function returns the already created instance.
 * must be called only after calling init 
 * atleast once before
 */
MemoryManager * MemoryManager::get()
{
    return instance;
}

/* function that creates the actual singleton instance */
MemoryManager* MemoryManager::init(bool isSharedMemory,
        uint64_t largeMemSize,
        uint64_t largeMemBlockSize,
        uint64_t smallMemSize,
        uint64_t smallMemBlockSize)
{
    if(!instance)
    {
        instance = new MemoryManager(isSharedMemory,
                largeMemSize,
                largeMemBlockSize,
                smallMemSize,
                smallMemBlockSize);
    }
    return instance;
}

void MemoryManager::destroy()
{
    if(instance)
    {
        delete instance;
    }
    instance = NULL;
}

/* wrapper function that creates the correct type of memory */
int MemoryManager::createMemory(size_t size,void *&mem)
{
    if(mIsSharedMemory)
        return createSharedMemory(size,mem);
    else
        return createHeapMemory(size,mem);
}

/* wrapper function that destroys the correct type of memory */
void MemoryManager::freeMemory(void *&mem,int id)
{
    if(mIsSharedMemory)
        return freeSharedMemory(mem,id);
    else
        freeHeapMemory(mem,id);
}

/*
 * To make sure there is no memory wastage, memory sizes are in 
 * terms of memory blocks.
 *
 * TODO: make this work on memory sizes and not block sizes 
 */
MemoryManager::MemoryManager(bool isSharedMemory,
        uint64_t largeMemSize, //in units of large mem blocks
        uint64_t largeMemBlockSize,
        uint64_t smallMemSize, //in units of small mem blocks
        uint64_t smallMemBlockSize) :
    mIsSharedMemory(isSharedMemory),
    mLargeMemSize(largeMemSize),
    mLargeMemBlockSize(largeMemBlockSize),
    mSmallMemSize(smallMemSize),
    mSmallMemBlockSize(smallMemBlockSize)

{
    /* create large memory with additional space for mem mgr upkeep */
    mLargeMemId = createMemory(
            (mLargeMemSize + (mLargeMemSize/mLargeMemBlockSize)) * 
            sizeof(MemoryHeader),mpLargeMemory);

    /* create small memory with additional space for mem mgr upkeep */
    mSmallMemId = createMemory(
            (mSmallMemSize + (mSmallMemSize/mSmallMemBlockSize)) * 
            sizeof(MemoryHeader),mpSmallMemory);

    /* initialize memory for usage */
    initializeMemory();
}

MemoryManager::~MemoryManager()
{
    /* destroy all the shared memories */
    freeMemory(mpLargeMemory,mLargeMemId);
    freeMemory(mpSmallMemory,mSmallMemId);
    mSmallMemId = 0;
    mLargeMemId = 0;
}

void MemoryManager::initializeMemory()
{
    /* intitialize large memory */
    MemoryHeader *memHdr = (MemoryHeader*)mpLargeMemory;
    memHdr->free = true;
    memHdr->count = mLargeMemSize/mLargeMemBlockSize;

    /* initialize small memory */
    memHdr = (MemoryHeader*)mpSmallMemory;
    memHdr->free = true;
    memHdr->count = mSmallMemSize/mSmallMemBlockSize;
}

int MemoryManager::createHeapMemory(size_t size,void *&mem)
{
    if((mem = malloc(size)) != NULL)
        return 1;
    return 0;
}

void MemoryManager::freeSharedMemory(void *&mem,int id)
{
    shmctl(id,IPC_RMID,NULL);
    mem = NULL;
}

void MemoryManager::freeHeapMemory(void *&mem,int id)
{
    free(mem);
    mem = NULL;
}

int MemoryManager::createSharedMemory(size_t size,void *&mem)
{
    int semId = 0;
    if((semId = shmget(IPC_PRIVATE,size,IPC_CREAT | 0666)) != -1)
    {

        if((mem = shmat(semId,NULL,0)) == (char *) -1)
        {
            return 0;
        }
    }
    return semId;
}

/*
 * allocated memory of the given size
 * checks for free memory blocks that fits the given size.
 * adjuscent blocks are combined to arrive at the req. number
 */
void* MemoryManager::memAlloc(size_t size)
{
    uint32_t t_size = 0,b_size = 0;
    uint32_t total = 0;
    uint16_t sem_num = -1;
    uint32_t cur = 0,avl = 0;
    uint32_t reqblk = 0;
    MemoryHeader *hdr = NULL;
    bool large_mem = false; //hold the type of mem to be used

    if(0 == size)
    {
        return NULL;
    }

    if(size > (mSmallMemBlockSize * sizeof(MemoryHeader)))
    {
        large_mem = true;
    }

    if(large_mem)
    {
        hdr = (MemoryHeader*)mpLargeMemory;
        t_size = mLargeMemSize;
        b_size = mLargeMemBlockSize;
        sem_num = 0;
    }
    else
    {
        hdr = (MemoryHeader*)mpSmallMemory;
        t_size = mSmallMemSize;
        b_size = mSmallMemBlockSize;
        sem_num = 1;
    }

    total = t_size/b_size;
    mLock.acquireLock();
    if(size % (b_size * sizeof(MemoryHeader)) > 0)
        reqblk = 1;
    reqblk += size/(b_size * sizeof(MemoryHeader));
    while(total > 0)
    {
        total -= hdr->count;
        if(hdr->free)
        {
            cur = hdr->count;
        }
        else
        {
            hdr += (hdr->count * (b_size + 1));
            avl = 0;
            continue;
        }
        if((avl + cur) >= reqblk)
        {
            int32_t extra = (avl + cur - reqblk);
            MemoryHeader * temp = hdr;

            if(extra != 0)
            {
                temp += ((cur - extra) * (b_size + 1));
                temp->free = true;
                temp->count = extra;
            }

            if(avl != 0)
            {
                hdr -= (avl * (b_size + 1));
            }

            hdr->free = false;
            hdr->count = reqblk;
            ++hdr;
            mLock.releaseLock();// allocated so release
            return (void *) hdr;
        }
        avl += cur;
        hdr += (hdr->count * (b_size + 1));
    }
    mLock.releaseLock();

    //we get here when we cannot allocate memory
    //throw MemoryFull(); //throw memory full exception
    return NULL;
}

/* free the given set of memory by setting its status */
void MemoryManager::memFree(void *location)
{
    MemoryHeader *temp = (MemoryHeader*) location;
    if(!temp)
    {
        return ;
    }
    --temp;
    temp->free = true;
}
