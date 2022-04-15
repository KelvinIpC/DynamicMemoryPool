#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_

#include <map>
#include <mutex>

#include "MemoryBlock.h"

class MemoryPool
{
public:
	MemoryPool(size_t size = 1 << 10);
	virtual ~MemoryPool();

	/*
	*
	*/
	MemoryBlock&& MemAlloc(size_t size);
	void MemDealloc(MemoryBlock& memBlock);

protected:
	void Insert(MemoryBlock&);
	bool Remove(MemoryBlock&);
	// MemoryBlock &&Find(uint32_t offset);


private:
	unsigned char* m_rawData;
	mutable std::mutex m_memoryBlockMtx;
	std::map<unsigned char*, MemoryBlock> m_memoryBlockMap;
};

#endif // MEMORY_POOL_H_
