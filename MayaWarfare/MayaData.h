#pragma once
#include <Windows.h>
#include "..\Maya\MayaToGameEngine\sharedMemory.h"
#include "..\Maya\MayaToGameEngine\SharedLayouts.h"

class MayaData{
public:
	MayaData();
	~MayaData();

	MessageType read();

	void getNewMesh(unsigned char UUID[UUIDSIZE], char*& name, VertexLayout*& vertecies, UINT& numVertecies, UINT*& indecies, UINT& numIndecies);
	void getNewCamera(unsigned char UUID[UUIDSIZE], char*& name, float camMatrix[4][4], bool* isortho);
	void getNewTransform(unsigned char UUID[UUIDSIZE], unsigned char shapeUUID[UUIDSIZE], ShapeType& shapeType, char*& name, char*& parentName, float translation[3], float scale[3], float rotation[4]);
	void getLight(float color[3], float& range);


	void getVertexChanged(unsigned char UUID[UUIDSIZE], VertexLayout*& verteciesData, UINT*& indexNumbers, UINT& numVerteciesChanged);

	void getRemoveNode(unsigned char UUID[UUIDSIZE], char*& name);

private:
	SharedMemory shared;
	char* data;
	INT64 dataSize;
};