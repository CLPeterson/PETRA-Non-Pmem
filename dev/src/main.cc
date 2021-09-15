#include <cstdlib>
#include <cstdio>
#include <thread>
#include "lockfreelist/lockfreelist.h"
#include "boostinglist/boostinglist.h"
using namespace std;

const int THREAD_COUNT = 2;

void runThread(LockfreeList* list, int threadId)
{
    list->Insert(threadId);
}

void runBoostingThread(BoostingList* list, int threadId)
{
    list->Init();

    BoostingList::ReturnCode ret;

    for(int i = 0; i < 2; i++)
    {
        ret = list->Insert(threadId+i);
        if(ret != BoostingList::OK)
        {
            list->OnAbort();
            return;
        }
    }

    list->OnCommit();
}

int main()
{
    LockfreeList* lockfreelist = new LockfreeList();

	thread threads[THREAD_COUNT]; // Create our threads
	for(long i=0; i<THREAD_COUNT; i++) { // Start our threads
		threads[i] = thread(runThread, lockfreelist, i);
	}
	for(int i=0; i<THREAD_COUNT; i++) { // Wait for all threads to complete
		threads[i].join();
	}

    lockfreelist->Print();


    BoostingList* boostinglist = new BoostingList();

	for(long i=0; i<THREAD_COUNT; i++) { // Start our threads
		threads[i] = thread(runBoostingThread, boostinglist, i);
	}
	for(int i=0; i<THREAD_COUNT; i++) { // Wait for all threads to complete
		threads[i].join();
	}

    boostinglist->Print();
}