#include <syslog.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include "MemoryManager.hpp"

#ifdef USE_SHARED_MEMORY
#define createMemory(size,mem) createSharedMemory(size,mem)
#define freeMemory(mem,id) freeSharedMemory(mem,id)
#else
#define createMemory(size,mem) createHeapMemory(size,mem)
#define freeMemory(mem,id) freeHeapMemory(mem,id)
#endif

void* memAlloc(size_t size)
{
    return MemoryManager::create()->memAlloc(size);
}

void memFree(void *mem)
{
    MemoryManager::create()->memFree(mem);
}

MemoryManager* MemoryManager::instance = NULL;

MemoryManager * MemoryManager::create()
{
	if(instance == NULL)
		instance = new MemoryManager();
	return instance;
}

void MemoryManager::destroy()
{
	if(instance)
		delete instance;
	instance = NULL;
}

MemoryManager::MemoryManager():mpSmallMemory(NULL),mpLargeMemory(NULL),
    mSmallMemId(0),mLargeMemId(0)
{
    /* create large memory */
	mLargeMemId = createMemory(
            (TOTAL_SIZE + (TOTAL_SIZE/BLOCK_SIZE)) * 
            sizeof(MemoryHeader),mpLargeMemory);

    /* create small memory */
	mSmallMemId = createMemory(
            (S_TOTAL_SIZE + (S_TOTAL_SIZE/S_BLOCK_SIZE)) * 
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
    memHdr->count = TOTAL_SIZE/BLOCK_SIZE;

    /* initialize small memory */
    memHdr = (MemoryHeader*)mpSmallMemory;
    memHdr->free = true;
    memHdr->count = S_TOTAL_SIZE/S_BLOCK_SIZE;
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

//allocate the given memory size
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

    if(size > (S_BLOCK_SIZE * sizeof(MemoryHeader)))
    {
        large_mem = true;
    }

    if(large_mem)
    {
        hdr = (MemoryHeader*)mpLargeMemory;
        t_size = TOTAL_SIZE;
        b_size = BLOCK_SIZE;
        sem_num = 0;
    }
    else
    {
        hdr = (MemoryHeader*)mpSmallMemory;
        t_size = S_TOTAL_SIZE;
        b_size = S_BLOCK_SIZE;
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

//free the given memory
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
