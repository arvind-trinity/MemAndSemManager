/* definition for shared memory manager */

#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include <stdint.h>
#include "SemaphoreManager.hpp"
#include "Lock.hpp"

/* TODO: define this based on DS requirement */
#define TOTAL_SIZE 1000000                                                         
#define BLOCK_SIZE 50
#define S_TOTAL_SIZE 140000
#define S_BLOCK_SIZE 7

//#define USE_SHARED_MEMORY 1

extern void* memAlloc(size_t);
extern void memFree(void*);

typedef struct
{
    uint32_t count;
    uint32_t free;
}MemoryHeader;

class MemoryManager
{
    public:
        static MemoryManager* create();
        static MemoryManager* create(uint64_t,uint64_t,uint64_t,uint64_t);
        static void destroy();
        void* memAlloc(size_t);
        void memFree(void*);

    private:
        MemoryManager();
        ~MemoryManager();
        int createSharedMemory(size_t,void*&);
        int createHeapMemory(size_t,void*&);
        void freeSharedMemory(void*&,int=0);
        void freeHeapMemory(void*&,int=0);
        void initializeMemory();

        void *mpSmallMemory;
        void *mpLargeMemory;
        int mSmallMemId;
        int mLargeMemId;
        Semaphore mSemaphore;
        static MemoryManager *instance;
        Lock mLock;
};

#endif
