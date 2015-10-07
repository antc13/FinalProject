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
	std::cout << "Initialize SharedMemory" << std::endl;
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
	std::cout << sizeof(SharedVars) << std::endl;
	assert(hFileMap != NULL);
	
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		isFirst = false;
		std::cout << "NOT first" << std::endl;
	} else
		std::cout << "first" << std::endl;

	mData = (char*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	sharedVars = (SharedVars*)((char*)mData + size);

	if (isFirst)
	{
		std::cout << "Initialize SharedVars" << std::endl;
		std::cout << "Shared Memory size: " << size << std::endl;
		sharedVars->freeMemory = size;
		sharedVars->head = 0;
		sharedVars->tail = 0;
	}
}

bool SharedMemory::Write(MessageType type, char* data, size_t length)
{
	std::cout << "Write" << std::endl;
	if (isProducer)
	{
		size_t msgLength = ((length + sizeof(MessageHeader)) / 4096 + 1) * 4096;
		if (msgLength <= sharedVars->freeMemory)
		{
			MessageHeader newMessageHeader;

			newMessageHeader.length = msgLength;
			newMessageHeader.padding = newMessageHeader.length - length - sizeof(MessageHeader);
			newMessageHeader.messageType = type;

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
			std::cout << newMessageHeader.length << " " << newMessageHeader.messageType << " " << newMessageHeader.padding << std::endl;
			std::cout << sharedVars->head << std::endl;
			std::cout << sharedVars->freeMemory << std::endl;
			return true;
		}
	}
	return false;
}

MessageType SharedMemory::Read(char*& returnData, size_t& returnDataLength, size_t& lengthOfMessage)
{
	if (!isProducer)
	{
		if (sharedVars->head != sharedVars->tail)
		{
			MessageHeader* msgHeader = (MessageHeader*)((char*)mData + sharedVars->tail);

			if (msgHeader->length - msgHeader->padding - sizeof(MessageHeader) > returnDataLength)
			{
				delete[] returnData;
				returnData = new char[msgHeader->length];
				returnDataLength = msgHeader->length;
			}
			ZeroMemory(returnData, returnDataLength);
			if (sharedVars->tail + msgHeader->length <= mSize)
			{
				memcpy(returnData, (char*)mData + sharedVars->tail + sizeof(MessageHeader), msgHeader->length - msgHeader->padding - sizeof(MessageHeader));
			}
			else
			{
				memcpy(returnData, (char*)mData + sharedVars->tail + sizeof(MessageHeader), mSize - (sharedVars->tail + sizeof(MessageHeader)));
				memcpy(&returnData[mSize - (sharedVars->tail + sizeof(MessageHeader))], mData, msgHeader->length - msgHeader->padding - sizeof(MessageHeader)-(mSize - (sharedVars->tail + sizeof(MessageHeader))));
			}
			sharedVars->tail = (sharedVars->tail + msgHeader->length) % mSize;
			sharedVars->freeMemory = sharedVars->freeMemory + msgHeader->length;

			lengthOfMessage = msgHeader->length - msgHeader->padding - sizeof(MessageHeader);

			return msgHeader->messageType;
		}
	}
	return mNoMessage;
}

