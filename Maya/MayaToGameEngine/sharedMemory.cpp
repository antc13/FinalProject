#include "sharedMemory.h"
#include <assert.h>

SharedMemory::SharedMemory()
{
	
}

SharedMemory::~SharedMemory()
{
	UnmapViewOfFile((LPCVOID)mData);
	CloseHandle(hFileMap);
	delete[] messageData;

}

void SharedMemory::initialize(DWORD size, LPCTSTR fileMapName, bool isProducer)
{
	bool isFirst = true;
	this->isProducer = isProducer;
	this->mSize = size;
	messageData = nullptr;
	messageDataSize = 0;

	this->hFileMap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		(DWORD)0,
		size + sizeof(SharedVars),
		(LPCTSTR)fileMapName);

	assert(hFileMap != NULL);

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		isFirst = false;
	}

	mData = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	sharedVars = (SharedVars*)((char*)mData + size);

	if (isFirst)
	{
		sharedVars->freeMemory = size;
		sharedVars->head = 0;
		sharedVars->tail = 0;
	}
}
#include <iostream>
bool SharedMemory::Write(MessageType type, void* data, UINT length)
{
	if (isProducer)
	{
		std::cout << "Hej" << std::endl;
		UINT msgLength = ((length + sizeof(MessageHeader)) / 4096 + 1) * 4096;
		if (msgLength <= sharedVars->freeMemory)
		{
			std::cout << "ojhf" << std::endl;
			MessageHeader newMessageHeader;

			newMessageHeader.length = msgLength;
			newMessageHeader.padding = newMessageHeader.length - length - sizeof(MessageHeader);

			memcpy((char*)mData + sharedVars->head, &newMessageHeader, sizeof(MessageHeader));

			if (sharedVars->head + newMessageHeader.length <= mSize)
			{
				memcpy((char*)mData + sharedVars->head + sizeof(MessageHeader), &data, length);
			}
			else
			{
				memcpy((char*)mData + sharedVars->head + sizeof(MessageHeader), data, mSize - (sharedVars->head + sizeof(MessageHeader)));
				memcpy(mData, (void*)((char*)data)[mSize - (sharedVars->head + sizeof(MessageHeader))], length - (mSize - (sharedVars->head + sizeof(MessageHeader))));
			}
			sharedVars->head = (sharedVars->head + msgLength) % mSize;
			sharedVars->freeMemory -= msgLength;
			std::cout << "oijfedewoirjf" << std::endl;
			return true;
		}
	}
	return false;
}

MessageType SharedMemory::Read(void* returnData, UINT& length)
{
	if (!isProducer)
	{
		if (sharedVars->head != sharedVars->tail)
		{
			MessageHeader* msgHeader = (MessageHeader*)((char*)mData + sharedVars->tail);

			if (msgHeader->length - msgHeader->padding - sizeof(MessageHeader) > messageDataSize)
			{
				delete[] messageData;
				messageData = new char[msgHeader->length];
				messageDataSize = msgHeader->length;
			}

			if (sharedVars->tail + msgHeader->length <= mSize)
			{
				memcpy((char*)mData + sharedVars->tail + sizeof(MessageHeader), &messageData, msgHeader->length - msgHeader->padding);
			}
			else
			{
				memcpy(messageData, (char*)mData + sharedVars->tail + sizeof(MessageHeader), mSize - (sharedVars->tail + sizeof(MessageHeader)));
				memcpy(&messageData[mSize - (sharedVars->tail + sizeof(MessageHeader))], mData, msgHeader->length - msgHeader->padding - sizeof(MessageHeader) - (mSize - (sharedVars->tail + sizeof(MessageHeader))));
			}
			sharedVars->tail = (sharedVars->tail + msgHeader->length) % mSize;
			sharedVars->freeMemory += msgHeader->length;

			returnData = messageData;
			length = msgHeader->length - msgHeader->padding - sizeof(MessageHeader);

			return msgHeader->messageType;

		}
	}
	return mNoMessage;
}

