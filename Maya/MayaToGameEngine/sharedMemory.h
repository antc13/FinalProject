#pragma once
#include <Windows.h>


struct TransformHeader
{
	INT64 itemNameLength;
};

class SharedMemory
{
public:
	SharedMemory();
	~SharedMemory();

	void initialize(DWORD size, LPCWSTR  fileMapName, bool isProducer = false);
	bool write(char* data, INT64 length);
	bool read(char*& returnData, INT64& returnDataLength);

private:
	struct SharedVars
	{
		INT64 head;
		INT64 tail;
		INT64 freeMemory;
	};

	struct MessageHeader
	{
		INT64 length;
		INT64 padding;
	};

	DWORD mSize;
	bool isProducer;
	HANDLE hFileMap;
	char* mData;
	SharedVars* sharedVars;
	
	//char* messageData;
	//size_t messageDataSize;
};