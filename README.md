MemAndSemManager
================

JAN 2009: Simple Memory and Semaphore (read and read/write locks) Manager.

Memory Manager:
    This can work with heap or shared memory and will work in 2 configurable block sizes. Based on the requested size memory is allocated from one the block store. Memory compaction (defragmentation) happens during every allocation.

    TODO: 
        Schedule de-fragmentation.
        Use more block sizes.
        Dynimically allocate incremental step sizes of the initial size. Option to enable or disable.

Semaphore Manager:
    Provides a simple mechanism to manupulate semaphores and use them as locks. Support for Read Lock, Read/Write Lock and Counter.

Examples:
    Usage examples are provided inside "exaples" folder.
        
