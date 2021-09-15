
#include "../lockfreelist/lockfreelist.h"
#include "dtx.h"

// static __thread UndoLog* log = NULL;

void UndoLog::Init()
{
    entries = new std::vector<LogEntry<boost::any>>();
}

template <typename T>
void UndoLog::Push(T* ptr, T oldData)
{
    entries->push_back(LogEntry<T>(ptr, oldData));
}

void UndoLog::Uninit()
{
    delete entries;
}