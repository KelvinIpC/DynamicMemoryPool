#include "MemoryBlock.h"
#include <iostream>

struct MemoryBlockData
{
	unsigned char* raw;
	size_t size;
	uint32_t refCnt;
	mutable std::mutex mtx;
	std::function<void(MemoryBlock&)> delocFunc;

	MemoryBlockData(unsigned char* data, size_t size, std::function<void(MemoryBlock&)> delocFunc)
		: raw(data)
		, size(size)
		, refCnt(1)
		, delocFunc(delocFunc)
	{}

	bool Valid() const
	{
		std::unique_lock<std::mutex> lck(mtx, std::try_to_lock);
		if (lck.owns_lock()) {
			return refCnt > 0;
		}
		else {
			return refCnt > 0;
		}
	}
};

MemoryBlock::MemoryBlock(unsigned char* data, size_t size, std::function<void(MemoryBlock&)> delocFunc)
	: m_dataPtr(std::make_shared<MemoryBlockData>(data, size, delocFunc))
{}

MemoryBlock::MemoryBlock(MemoryBlock&& memBlock)
	: m_dataPtr(memBlock.m_dataPtr)
{
	Retain();
}

MemoryBlock::MemoryBlock(const MemoryBlock& memBlock)
{
	m_dataPtr = memBlock.m_dataPtr;
	Retain();
}

MemoryBlock::~MemoryBlock()
{
	std::cout << "~MemoryBlock, ref:" << Reference() << std::endl;
	if (m_dataPtr != nullptr && m_dataPtr->Valid()) {
		std::lock_guard<std::mutex> lck(m_dataPtr->mtx);
		--m_dataPtr->refCnt;
		if (m_dataPtr->refCnt == 0) {
			Release();
		}
	}
}

size_t MemoryBlock::Size() const
{
	if (m_dataPtr != nullptr && m_dataPtr->Valid()) {
		std::lock_guard<std::mutex> lck(m_dataPtr->mtx);
		return m_dataPtr->size;
	}
	else {
		return 0;
	}
}

uint32_t MemoryBlock::Reference() const
{
	if (m_dataPtr != nullptr && m_dataPtr->Valid()) {
		std::lock_guard<std::mutex> lck(m_dataPtr->mtx);
		return m_dataPtr->refCnt;
	}
	else {
		return 0;
	}
}

void MemoryBlock::Retain()
{
	if (m_dataPtr != nullptr && m_dataPtr->refCnt > 0) {
		std::lock_guard<std::mutex> lck(m_dataPtr->mtx);
		++m_dataPtr->refCnt;
	}
}

void MemoryBlock::Release()
{
	if (m_dataPtr == nullptr)
	{
		return;
	}

	std::unique_lock<std::mutex> lck(m_dataPtr->mtx, std::try_to_lock);
	if (lck.owns_lock()) {
		--m_dataPtr->refCnt;
		if (m_dataPtr->refCnt == 0) {

			if (m_dataPtr->delocFunc) {
				(m_dataPtr->delocFunc)(*this);
				m_dataPtr->delocFunc = nullptr;
			}

			m_dataPtr->raw = nullptr;
			m_dataPtr->size = 0;
		}
		else if (m_dataPtr->refCnt < 0) {
			assert(false); // TBD crash
		}
	}
}

MemoryBlock MemoryBlock::Crop(size_t size)
{
	std::lock_guard<std::mutex> lck(m_dataPtr->mtx);
	if (size == this->m_dataPtr->size) {
		std::cout << "hehe" << std::endl;
		return *this;
	}
	else if (size < this->m_dataPtr->size) {
		std::cout << "hehe1" << std::endl;
		this->m_dataPtr->size -= size; 
		MemoryBlock mem(this->m_dataPtr->raw + this->m_dataPtr->size, size, this->m_dataPtr->delocFunc);
		return mem;
	}
	else {
		assert(false);
	}
}

void MemoryBlock::Merge(MemoryBlock &memBlock)
{
	std::lock(m_dataPtr->mtx, memBlock.m_dataPtr->mtx);
	if (memBlock.m_dataPtr->raw == (this->m_dataPtr->raw + this->m_dataPtr->size)) {
		this->m_dataPtr->size += memBlock.m_dataPtr->size;
		// memBlock.Release();
	}
	else if (this->m_dataPtr->raw == (memBlock.m_dataPtr->raw + memBlock.m_dataPtr->size)) {
		this->m_dataPtr->raw = memBlock.m_dataPtr->raw;
		this->m_dataPtr->size += memBlock.m_dataPtr->size;
		// memBlock.Release();
	}
	else {
		assert(false); // TBD crash
	}
	
}

unsigned char* MemoryBlock::Raw()
{
	return m_dataPtr->raw;
}
