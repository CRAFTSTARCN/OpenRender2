#pragma once

class AutoIncreaseIdAllocator
{
	size_t Current = 0;
	
public:

	AutoIncreaseIdAllocator(size_t Start);

	size_t GetNewId();
	
};
