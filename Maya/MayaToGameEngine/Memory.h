#pragma once

class Memory
{
public:
	Memory();
	~Memory();
	char* getAllocatedMemory(size_t size);
	size_t& getSize();

private:
	char* d;
	size_t d_size;
};
