#include "alloc.h"
#include "memorymanager.h"
void* operator new(size_t size)
{
	return MemoryManager::instance().AllocMemory(size);
}

void* operator new[](size_t size)
{
	return MemoryManager::instance().AllocMemory(size);
}

void operator delete(void* ptr)
{
	MemoryManager::instance().FreeMemory(ptr);
}

void operator delete[](void* ptr)
{
	MemoryManager::instance().FreeMemory(ptr);
}
