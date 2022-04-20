#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H
#include <stdlib.h>
#include <assert.h>
#include <mutex>
template <class Type, size_t nPoolSzie>
class ObjectPool
{
public:
	ObjectPool()
	{
		initPool();
	}

	~ObjectPool()
	{
		if (_pBuf)
			delete[] _pBuf;
	}

private:
	class NodeHeader
	{
	public:
		NodeHeader *pNext;
		int nID;
		char nRef;
		bool bPool;

	private:
		char c1;
		char c2;
	};

public:
	void freeObjMemory(void *pMem)
	{
		NodeHeader *pBlock = (NodeHeader *)((char *)pMem - sizeof(NodeHeader));
		assert(1 == pBlock->nRef);
		if (pBlock->bPool)
		{
			std::lock_guard<std::mutex> lg(_mutex);
			if (--pBlock->nRef != 0)
			{
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else
		{
			if (--pBlock->nRef != 0)
			{
				return;
			}
			delete[] pBlock;
		}
	}

	void *allocObjMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		NodeHeader *pReturn = nullptr;
		if (nullptr == _pHeader)
		{
			pReturn = (NodeHeader *)new char[sizeof(Type) + sizeof(NodeHeader)];
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
		}
		else
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		return ((char *)pReturn + sizeof(NodeHeader));
	}

private:
	void initPool()
	{
		assert(nullptr == _pBuf);
		if (_pBuf)
			return;
		size_t realSzie = sizeof(Type) + sizeof(NodeHeader);
		size_t n = nPoolSzie * realSzie;
		_pBuf = new char[n];
		_pHeader = (NodeHeader *)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;
		NodeHeader *pTemp1 = _pHeader;
		for (size_t n = 1; n < nPoolSzie; n++)
		{
			NodeHeader *pTemp2 = (NodeHeader *)(_pBuf + (n * realSzie));
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}

private:
	NodeHeader *_pHeader;
	char *_pBuf;
	std::mutex _mutex;
};

template <class Type, size_t nPoolSzie>
class ObjectPoolBase
{
public:
	void *operator new(size_t nSize)
	{
		return objectPool().allocObjMemory(nSize);
	}

	void operator delete(void *p)
	{
		objectPool().freeObjMemory(p);
	}

	template <typename... Args>
	static Type *createObject(Args... args)
	{
		Type *obj = new Type(args...);
		return obj;
	}

	static void destroyObject(Type *obj)
	{
		delete obj;
	}

private:
	typedef ObjectPool<Type, nPoolSzie> ClassTypePool;
	static ClassTypePool &objectPool()
	{
		static ClassTypePool sPool;
		return sPool;
	}
};

#endif
