#include "AutoIncreaseIdAllocator.h"

AutoIncreaseIdAllocator::AutoIncreaseIdAllocator(size_t Start) : Current(Start) 
{
}

size_t AutoIncreaseIdAllocator::GetNewId()
{
	return Current++;
}
