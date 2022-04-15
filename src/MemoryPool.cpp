#include "MemoryPool.h"

#include <iostream>
MemoryPool::MemoryPool(size_t size)
{
	m_rawData = new unsigned char[size];
	MemoryBlock initBlock(m_rawData, size, std::bind(&MemoryPool::MemDealloc, this, std::placeholders::_1));

	std::cout << "haha" << std::endl;
	std::lock_guard<std::mutex> lck(m_memoryBlockMtx);
	m_memoryBlockMap.insert(std::make_pair(m_rawData, initBlock));
}

MemoryPool::~MemoryPool()
{
	std::lock_guard<std::mutex> lck(m_memoryBlockMtx);
	std::cout << "~MemoryPool, size:" << m_memoryBlockMap.size() << std::endl;
	m_memoryBlockMap.clear();
	if (m_rawData != nullptr) {
		delete m_rawData;
		m_rawData = nullptr;
	}
}

MemoryBlock&& MemoryPool::MemAlloc(size_t size)
{
	std::lock_guard<std::mutex> lck(m_memoryBlockMtx);
	for (auto it = m_memoryBlockMap.begin(); it != m_memoryBlockMap.end(); ++it) {
		if (it->second.Size() > size) {
			std::cout << "haha1" << std::endl;
			auto mb = it->second.Crop(size);
			std::cout << "haha" << std::endl;
			m_memoryBlockMap.insert(std::make_pair(mb.Raw(), std::move(mb)));
			return std::move(mb);
		}
		else if (it->second.Size() == size) {
			return std::move(it->second);
		}
	}

	std::cout << "haha" << std::endl;
	MemoryBlock mb(nullptr, 0, nullptr);
	return std::move(mb);
}

void MemoryPool::MemDealloc(MemoryBlock& memBlock)
{
	std::lock_guard<std::mutex> lck(m_memoryBlockMtx);

	auto it = m_memoryBlockMap.find(memBlock.Raw());
	if (it != m_memoryBlockMap.end()) {
		// merge the previous block
		auto itPrevious = it;
		auto itFollowing = it;
		--itPrevious;
		if (itPrevious != m_memoryBlockMap.end()) {
			if (itPrevious->second.Reference() == 0) {
				memBlock.Merge(itPrevious->second);
			}
		}

		// merge the following block
		--itFollowing;
		if (itFollowing != m_memoryBlockMap.end()) {
			if (itPrevious->second.Reference() == 0) {
				memBlock.Merge(itFollowing->second);
			}
		}
	}
	else {
		assert(false);
	}
}

void MemoryPool::Insert(MemoryBlock& memBlock)
{
	std::lock_guard<std::mutex> lck(m_memoryBlockMtx);
	m_memoryBlockMap.insert(std::make_pair(memBlock.Raw(), std::move(memBlock)));
	// m_memoryBlockMap[memBlock.Raw()] = std::move(memBlock);
}

bool MemoryPool::Remove(MemoryBlock& memBlock)
{
	std::lock_guard<std::mutex> lck(m_memoryBlockMtx);
	auto it = m_memoryBlockMap.find(memBlock.Raw());
	if (it != m_memoryBlockMap.end()) {
		m_memoryBlockMap.erase(it);
		return true;
	}
	else {
		return false;
	}
}
