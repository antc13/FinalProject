#include "sharedMemory.h"
#include <assert.h>
#include <iostream>

SharedMemory::SharedMemory()
{
	
}

SharedMemory::~SharedMemory()
{
	UnmapViewOfFile((LPCVOID)mData);
	CloseHandle(hFileMap);
}

void SharedMemory::initialize(DWORD size, LPCWSTR  fileMapName, bool isProducer)
{
	bool isFirst = true;
	this->isProducer = isProducer;
	this->mSize = size;

	this->hFileMap = CreateFileMappingW(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		(DWORD)0,
		size + sizeof(SharedVars),
		fileMapName);
	
	assert(hFileMap != NULL);
	
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		isFirst = false;

	mData = (char*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	sharedVars = (SharedVars*)((char*)mData + size);

	if (isFirst)
	{
		sharedVars->freeMemory = size;
		sharedVars->head = 0;
		sharedVars->tail = 0;
	}
}

bool SharedMemory::write(char* data, INT64 length)
{
	if (isProducer)
	{
		INT64 msgLength = ((length + sizeof(MessageHeader)) / 4096 + 1) * 4096;
		if (msgLength <= sharedVars->freeMemory)
		{
			MessageHeader newMessageHeader;

			newMessageHeader.length = msgLength;
			newMessageHeader.padding = newMessageHeader.length - length - sizeof(MessageHeader);

			memcpy(mData + sharedVars->head, &newMessageHeader, sizeof(MessageHeader));

			if (sharedVars->head + newMessageHeader.length <= mSize)
			{
				memcpy(mData + sharedVars->head + sizeof(MessageHeader), data, length);
			}
			else
			{
				memcpy(mData + sharedVars->head + sizeof(MessageHeader), data, mSize - (sharedVars->head + sizeof(MessageHeader)));
				memcpy(mData, &data[mSize - (sharedVars->head + sizeof(MessageHeader))], length - (mSize - (sharedVars->head + sizeof(MessageHeader))));
			}
			sharedVars->head = (sharedVars->head + msgLength) % mSize;
			sharedVars->freeMemory = sharedVars->freeMemory - msgLength;
			return true;
		}
	}
	return false;
}

bool SharedMemory::read(char*& returnData, INT64& returnDataLength)
{
	if (!isProducer)
	{
		if (sharedVars->head != sharedVars->tail)
		{
			MessageHeader* msgHeader = (MessageHeader*)((char*)mData + sharedVars->tail);

			if (msgHeader->length - msgHeader->padding - sizeof(MessageHeader) > returnDataLength)
			{
				delete[] returnData;
				returnData = new char[msgHeader->length - msgHeader->padding - sizeof(MessageHeader)];
				returnDataLength = msgHeader->length - msgHeader->padding - sizeof(MessageHeader);
			}
			//ZeroMemory(returnData, returnDataLength);
			if (sharedVars->tail + msgHeader->length <= mSize)
			{
				memcpy(returnData, mData + sharedVars->tail + sizeof(MessageHeader), msgHeader->length - msgHeader->padding - sizeof(MessageHeader));
			}
			else
			{
				memcpy(returnData, mData + sharedVars->tail + sizeof(MessageHeader), mSize - (sharedVars->tail + sizeof(MessageHeader)));
				memcpy(&(returnData)[mSize - (sharedVars->tail + sizeof(MessageHeader))], mData, msgHeader->length - msgHeader->padding - sizeof(MessageHeader)-(mSize - (sharedVars->tail + sizeof(MessageHeader))));
			}
			sharedVars->tail = (sharedVars->tail + msgHeader->length) % mSize;
			sharedVars->freeMemory = sharedVars->freeMemory + msgHeader->length;

			return true;
		}
	}
	return false;
}

