MemAndSemManager
================

JAN 2009: Simple Memory and Semaphore (read and read/write locks) Manager.

The idea is to provide a simple solution to most common problem in C++ programming, memory management and synchroniztion. The project does not claim to be the ultimate solution or not even as usable, but a simple step towards easy programing to avoid reinventing the wheel. Please forwards any bugs or improvement suggestions to arvind.trinity@gmail.com.

There are 3 main components in the project
(1) Memory Manager
(2) Semaphore Manager
(3) Locks based on Semaphore Manager

Memory Manager:
    A simple way to handle all the memory that your program will ever need esp useful for memory hungry programs. This helps in making sure that the memory required by your program is pre-reserved and is available to use. You can decide on a bare minimum size and the manager will expand the memory dynimically based on need. Can work on both heap and shared memories and operates in similar manner as malloc or free. 

    Usage:
        MemoryManager::init(<2 sizes in blocks of 64bit>) //initialize the memory manager
        MemoryManager::destroy() //destroys and frees all resources used by memory manager
        memAlloc(<size>) //alloctes memory of given size
        memFree(<memory>) //frees memory given

Semaphore Manager:
    Provides a simple mechanism to create, destroy and manupulate semaphores. Creates a semaphore array and returns single or semaphore pair when asked. Maintains the current index of the first free semaphore and a list of previously freed semaphores to improve preformance. Returns a semaphore object that can be used as a counter or a lock.

    Usage:
        SemaphoreManager::create()
        getSemaphore();
        getSemaphorePair();
        freeSemaphore();
        freeSemaphorePair();

Locks:
Provides both mutex and read/write lock. Mutex is when a lock is either locked or unlocked. In read write lock, there can be more than one read operation happening on the protected resource but only one write operation at a time. Also read and write are mutually exclusive.

    Usage:
        lock.acquireLock()
        lock.releaseLock()
        rwLock.acquireReadLock()
        reLock.releaseReadLock()
        rwLock.acquireWriteLock()
        rwlock.releaseWriteLock()



