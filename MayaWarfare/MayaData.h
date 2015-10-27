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
	void getNewCamera(char*& name, float camMatrix[4][4]/*, bool* isortho, float* nearPlane, float* farPlane, float* aspectRatio, float* fov*/);
	void getCameraChanged(char*& name);
	void getNewTransform(char*& name, float translation[3], float scale[3], float rotation[4]);
	void getLight(float color[3], float& range);
	void getMaterial(char*& name, char*& texturePath, float diffuseColor[3], float ambientColor[3]);
	void getMeshMaterialNames(char*& meshName, char*& materialName);

	void getVertexChanged(char*& name, VertexLayout*& verteciesData, UINT*& indexNumbers, UINT& numVerteciesChanged);

	void getRemoveNode(char*& name);

private:
	SharedMemory shared;
	char* data;
	INT64 dataSize;
};