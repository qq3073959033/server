#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <mutex>
#include <functional>
#include <unordered_map>
using namespace std;
class MemoryPool;
class MemoryBlock
{
public:
	int nId_;
	int nRef_;
	bool bPool_;
	MemoryPool *pPool_;
	MemoryBlock *pNext_;
};

class MemoryPool
{
public:
	MemoryPool(size_t blockSize, size_t blockCount)
		: pPool_(nullptr)
		, blockSize_(blockSize)
		, blockCount_(blockCount)
		, freeBlock_(nullptr)
		, memoryLen_((sizeof(MemoryBlock) + blockSize_) * blockCount_)
	{
		initMemory();
	}

	void initMemory()
	{
		if (pPool_ != nullptr)
		{
			return;
		}
		pPool_ = (char *)malloc((blockSize_ + sizeof(MemoryBlock)) * blockCount_);
		if (pPool_ == nullptr)
		{
			std::cout << "the memory is empty" << std::endl;
			return;
		}
		MemoryBlock *pBlock = (MemoryBlock *)pPool_;
		freeBlock_ = pBlock;
		MemoryBlock *lastBlock = (MemoryBlock *)((char *)pPool_ + memoryLen_);
		int i = 0;
		for (; pBlock < lastBlock;)
		{
			pBlock->bPool_ = true;
			pBlock->nId_ = i++;
			pBlock->nRef_ = 0;
			pBlock->pPool_ = this;
			pBlock->pNext_ = (MemoryBlock *)(((char *)pBlock) + (blockSize_ + sizeof(MemoryBlock)));
			pBlock = (MemoryBlock *)(((char *)pBlock) + (blockSize_ + sizeof(MemoryBlock)));
		}
		pBlock = (MemoryBlock *)(((char *)pBlock) - (blockSize_ + sizeof(MemoryBlock)));
		pBlock->pNext_ = nullptr;
	}

	void *getMemoryBlock(size_t size)
	{
		lock_guard<mutex> lock(mtx_);
		if (freeBlock_ == nullptr)
		{
			MemoryBlock *pBlock = (MemoryBlock *)malloc(blockSize_ + sizeof(MemoryBlock));
			if (pBlock == nullptr)
			{
				throw "the memory is empty";
			}
			pBlock->nId_ = -1;
			pBlock->nRef_ = 1;
			pBlock->pPool_ = nullptr;
			pBlock->pNext_ = nullptr;
			pBlock->bPool_ = false;
			return pBlock + 1;
		}
		else
		{
			MemoryBlock *freeBlock = freeBlock_;
			freeBlock_->nRef_++;
			freeBlock_ = freeBlock_->pNext_;
			return freeBlock + 1;
		}
	}

	void test()
	{
		MemoryBlock *pBlock = (MemoryBlock *)pPool_;
		for (int i = 0; i < blockCount_; ++i)
		{
			cout << (((char *)pBlock) + (blockSize_ + sizeof(MemoryBlock))) - (char *)pBlock << endl;
			pBlock = (MemoryBlock *)(((char *)pBlock) + (blockSize_ + sizeof(MemoryBlock)));
		}
	}

	void rebackMemoryBlock(MemoryBlock *pBlcok)
	{
		lock_guard<mutex> lock(mtx_);
		pBlcok->pNext_ = freeBlock_;
		freeBlock_ = pBlcok;
	}

private:
	size_t blockSize_;
	size_t blockCount_;
	char *pPool_;
	MemoryBlock *freeBlock_;
	mutex mtx_;
	size_t memoryLen_;
};

template <unsigned blockSize,unsigned blockCount>
class InitPool : public MemoryPool
{
public:
	InitPool()
		: MemoryPool(blockSize, blockCount)
	{
	}
};

class MemoryManager
{
public:
	~MemoryManager()
	{
	}

	static MemoryManager &instance()
	{
		static MemoryManager manger;
		return manger;
	}

	void *AllocMemory(size_t size)
	{
		// 优化：可以用映射的方式去
		// 哈希表的时间复杂度为O(1)
		// 查找更方便！
		if (size <= 64)
		{
			return pool1_.getMemoryBlock(size);
		}
		else if (64 < size <= 128)
		{
			return pool2_.getMemoryBlock(size);
		}
		else if (128 < size <= 256)
		{
			return pool3_.getMemoryBlock(size);
		}
		else if (256 < size <= 512)
		{
			return pool4_.getMemoryBlock(size);
		}
		return pool1_.getMemoryBlock(size);
	}

	void FreeMemory(void *ptr)
	{
		MemoryBlock *pBlock = ((MemoryBlock *)ptr) - 1;
		assert(--(pBlock->nRef_) == 0);
		if (pBlock->bPool_ == true)
		{
			pBlock->pPool_->rebackMemoryBlock(pBlock);
		}
		else
		{
			free(pBlock);
		}
	}

	// 测试接口
	void test()
	{
		pool1_.test();
	}

private:
	MemoryManager()
	{
	}
	InitPool<64, 1000000> pool1_;
	InitPool<128, 100> pool2_;
	InitPool<258, 100> pool3_;
	InitPool<512, 100> pool4_;
};

#endif