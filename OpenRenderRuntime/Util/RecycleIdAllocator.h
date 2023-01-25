#pragma once
#include <stack>
#include <unordered_set>

class RecycleIdAllocator
{
	
	size_t Current;
	std::stack<size_t> Reuse;
	std::unordered_set<size_t> Allocated;
	
public:

	RecycleIdAllocator(size_t Start);

	size_t GetNewId();
	void DeallocateId(size_t Id);
};
