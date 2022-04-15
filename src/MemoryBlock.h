#ifndef MEMORY_BLOCK_H_
#define MEMORY_BLOCK_H_

#include <memory>
#include <functional>

struct MemoryBlockData;

class MemoryBlock
{
public:
	MemoryBlock(unsigned char*, size_t, std::function<void(MemoryBlock&)>);
	MemoryBlock(MemoryBlock&&);
	MemoryBlock(const MemoryBlock&);
	virtual ~MemoryBlock();

	size_t Size() const;
	uint32_t Reference() const;
	void Retain();
	void Release();

	MemoryBlock Crop(size_t size);
	void Merge(MemoryBlock&);

	unsigned char* Raw();

private:
	std::shared_ptr<MemoryBlockData> m_dataPtr;
};

#endif // MEMORY_BLOCK_H_
