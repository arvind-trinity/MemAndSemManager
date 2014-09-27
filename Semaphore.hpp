#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

class Semaphore
{
    public:
        Semaphore(int,int);
        Semaphore();
        int getCount();
        int setCount(int);
        int increaseCount();
        int decreaseCount();
        int getId(){return mId;}
        int getIndex(){return mIndex;}
        void setId(int id){mId = id;}
        void setIndex(int index){mIndex = index;}

    private:
        int mId;
        int mIndex;

        friend class SemaphoreManager;
};

#endif
