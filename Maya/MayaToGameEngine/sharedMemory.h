#pragma once
#include <Windows.h>

enum MessageType{ mNoMessage = 0, mNewMesh, mLight, mCamera, mTransform, mMaterial, mVertexChange, mNameChange };
struct MeshHeader
{
	UINT nameLength;
	UINT vertexCount;
};

class SharedMemory
{
public:
	SharedMemory();
	~SharedMemory();

	void initialize(DWORD size, LPCTSTR fileMapName, bool isProducer = false);

	bool Write(MessageType type, void* data, UINT length);
	MessageType Read(void* returnData, UINT& length);

private:
	struct SharedVars
	{
		size_t head;
		size_t tail;
		size_t freeMemory;
	};

	struct MessageHeader
	{
		size_t length;
		size_t padding;
		MessageType messageType;
	};

	DWORD mSize;
	bool isProducer;
	HANDLE hFileMap;
	void* mData;
	SharedVars* sharedVars;
	
	char* messageData;
	UINT messageDataSize;
};

extern SharedMemory gShared;