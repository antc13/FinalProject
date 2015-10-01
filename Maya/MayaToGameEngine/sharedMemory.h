#pragma once
#include <Windows.h>

enum MessageType{ mNoMessage = 0, mNewMesh, mLight, mCamera, mTransform, mMaterial, mVertexChange, mNameChange };
struct MeshHeader
{
	size_t nameLength;
	size_t vertexCount;
};

struct VertexLayout
{
	float pos[3];
};

class SharedMemory
{
public:
	SharedMemory();
	~SharedMemory();

	void initialize(DWORD size, LPCWSTR  fileMapName, bool isProducer = false);

	bool Write(MessageType type, void* data, size_t length);
	MessageType Read(void*& returnData, size_t& length);

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
	size_t messageDataSize;
};