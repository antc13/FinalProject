#include "Memory.h"

Memory::Memory()
{
	d = nullptr;
	d_size = 0;
}

Memory::~Memory()
{
	delete[] d;
}

char* Memory::getAllocatedMemory(size_t size)
{
	if (size > d_size)
	{
		delete[] d;
		d = new char[size];
	}
	return d;
}

size_t& Memory::getSize()
{
	return d_size;
}