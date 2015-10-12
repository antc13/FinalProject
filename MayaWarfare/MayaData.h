#pragma once
#include <Windows.h>
#include "..\Maya\MayaToGameEngine\sharedMemory.h"
#include "..\Maya\MayaToGameEngine\SharedLayouts.h"

class MayaData{
public:
	MayaData();
	~MayaData();

	MessageType read();

	void getNewMesh(char*& name, VertexLayout*& vertecies, UINT& numVertecies, UINT*& indecies, UINT& numIndecies);
	void getNewCamera(float camMatrix[4][4]);
	void getNewTransform(char*& name, float translation[3], float scale[3], float rotation[4]);

	void getVertexChanged(char*& name, VertexLayout*& verteciesData, UINT*& indexNumbers, UINT& numVerteciesChanged);

	void getRemoveNode(char*& name);

private:
	SharedMemory shared;
	char* data;
	INT64 dataSize;
};