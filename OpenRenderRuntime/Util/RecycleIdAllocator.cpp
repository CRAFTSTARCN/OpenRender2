#include "RecycleIdAllocator.h"

RecycleIdAllocator::RecycleIdAllocator(size_t Start) : Current(Start)
{
}

size_t RecycleIdAllocator::GetNewId()
{
	if(Reuse.empty())
	{
		return Current++;
	}

	size_t Ans = Reuse.top();
	Reuse.pop();
	return Ans;
}

void RecycleIdAllocator::DeallocateId(size_t Id)
{
	if(auto Iter = Allocated.find(Id); Iter == Allocated.end())
	{
		return;
	}
	else
	{
		Allocated.erase(Iter);
		Reuse.push(Id);
	}
}
