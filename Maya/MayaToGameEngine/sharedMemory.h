#pragma once
#include <Windows.h>

enum MessageType{ mNoMessage = 0, mNewMesh, mLight, mCamera, mTransform, mMaterial, mVertexChange, mNameChange };
struct MeshHeader
{
	INT64 nameLength;
	INT64 vertexCount;
	INT64 indexCount;
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

	bool Write(MessageType type, char* data, INT64 length);
	MessageType Read(char** returnData, INT64& returnDataLength, INT64& length);

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
		MessageType messageType;
	};

	DWORD mSize;
	bool isProducer;
	HANDLE hFileMap;
	char* mData;
	SharedVars* sharedVars;
	
	//char* messageData;
	//size_t messageDataSize;
};