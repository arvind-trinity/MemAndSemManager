/* declaration of semaphore manager */

#ifndef SEMAPHORE_MANAGER_HPP
#define SEMAPHORE_MANAGER_HPP

#include <stdint.h>
#include <sys/sem.h>
#include <list>
#include <map>
#include "Semaphore.hpp"

#define MAXIMUM_SEMAPHORE_COUNT 40
#define SEM_DETAILS_LOCATION "/proc/sys/kernel/sem"

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

        /* semaphore lists */
        Semaphore mCurrentSemArray;
        SemaphoreArray mFreeSemList;

        /* helper functions */
        int getSemaphoreArray();
        void freeSemaphoreArray(int);

        /* semaphore counts and limits */
        int mMaxSemInArray;
        int mMaxSemArray;

        static SemaphoreManager *mpInstance;

        friend class Semaphore;
};

#endif
