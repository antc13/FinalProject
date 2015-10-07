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

char*& Memory::getAllocatedMemory(INT64 size)
{
	if (d_size < size)
	{
		delete[] d;
		d = new char[size];
	}
	return d;
}

INT64& Memory::getSize()
{
	return d_size;
}