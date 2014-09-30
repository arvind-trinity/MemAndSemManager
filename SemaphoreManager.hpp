/* 
 * creates and maintains a list of semaphores
 * and provides functions that can create or 
 * destroy one or a pair of semaphores
 */

#ifndef SEMAPHORE_MANAGER_HPP
#define SEMAPHORE_MANAGER_HPP

#include <stdint.h>
#include <sys/sem.h>
#include <list>
#include <map>
#include "Lock.hpp"
#include "Semaphore.hpp"

#define SEM_DETAILS_LOCATION "/proc/sys/kernel/sem"

/* holds a pair of semaphore */
typedef struct
{
    int id;
    int firstIndex;
    int secondIndex;
}SemaphorePair;

typedef std::map<int,std::list<int> > SemaphoreArray;
typedef std::map<int,std::list<int> >::iterator SemaphoreArrayItr;

class SemaphoreManager
{
    public:
        static SemaphoreManager* create();
        static void destroy();
        Semaphore getSemaphore();
        SemaphorePair getSemaphorePair();
        void freeSemaphore(Semaphore);
        void freeSemaphorePair(SemaphorePair);

    private:
        SemaphoreManager();
        ~SemaphoreManager();

        /* initialize functions */
        int initializeSemaphoreSize();
        int initializeSemaphoreList();

        /* helper functions */
        int getSemaphoreArray();
        void freeSemaphoreArray(int);

        /* semaphore lists */
        Semaphore mCurrentSemArray; //holds the current free position in the list
        SemaphoreArray mFreeSemList; //holds all the semaphores that are free

        /* semaphore counts and limits */
        int mMaxSemInArray;
        int mMaxSemArray;

        /* lock for sync */
        Lock *mpLock;

        static SemaphoreManager *mpInstance;

        friend class Semaphore;
};

#endif
