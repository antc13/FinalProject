#pragma once
#include <Windows.h>

class Memory
{
public:
	Memory();
	~Memory();
	char*& getAllocatedMemory(INT64 size = 0);
	INT64& getSize();

private:
	char* d;
	INT64 d_size;
};
