/* 
 * definition for memory manager 
 * works on both shared memory or heap memory
 * helps in reducing fagamentation and make sure the
 * required memory to run a process is available at the 
 * beginning of the program.
 *
 * works in blocks of each 64bit in size. The requested amount 
 * in allocated and is initialized at the beginning. before each memory there 
 * is a status block that holds wheter blocks are free or used and 
 * how many number of them are in this set.
 * If the given size cannot be allocated then adjesent blocks are combined.
 * Memory compaction happens during each allocation and only when needed
 *
 * TODO:
 *  scheduled memory compaction instead of as need.
 *  use more than 2 block sizes to provide more dynamic approcah
 *  dynamically allocate more memory when the initially allocated is all
 *      used up. Can be enabled or disabled during creation
 *  throw exceptions when memory is full or during failure.
 */

#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include <stdint.h>
#include "Lock.hpp"

extern void* memAlloc(size_t);
extern void memFree(void*);

/* basic memory unit */
typedef struct
{
    uint32_t count;
    uint32_t free;
}MemoryHeader;

class MemoryManager
{
    public:
        static MemoryManager* get(); //default create functon
        static MemoryManager* init(bool,uint64_t,uint64_t,uint64_t,uint64_t);
        static void destroy();
        void* memAlloc(size_t);
        void memFree(void*);

    private:
        MemoryManager(bool,uint64_t,uint64_t,uint64_t,uint64_t);
        ~MemoryManager();
        void initializeMemory();

        /* create and destroy functions */
        int createSharedMemory(size_t,void*&);
        int createHeapMemory(size_t,void*&);
        void freeSharedMemory(void*&,int=0);
        void freeHeapMemory(void*&,int=0);
        int createMemory(size_t,void*&);
        void freeMemory(void*&,int=0);

        /* memory holders */
        void *mpSmallMemory;
        void *mpLargeMemory;

        /* shared memory ids */
        int mSmallMemId;
        int mLargeMemId;

        /* size and type holders */
        bool mIsSharedMemory;
        uint64_t mLargeMemSize, mLargeMemBlockSize;
        uint64_t mSmallMemSize, mSmallMemBlockSize;

        /* lock for sync */
        Lock mLock;

        /* static instance for single-ton implementation */
        static MemoryManager *instance;
};

#endif
