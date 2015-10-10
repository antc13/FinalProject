#pragma once
#include <Windows.h>
#include "..\Maya\MayaToGameEngine\sharedMemory.h"
#include "..\Maya\MayaToGameEngine\SharedLayouts.h"

class MayaData{
public:


	MayaData();
	~MayaData();

	MessageType read();
	void getNewMesh(char*& name, VertexLayout*& vertecies, UINT64& numVertecies, UINT*& indecies, UINT64& numIndecies);
	void getNewTransform(char*& name, float*& translation, float*& scale, float*& rotation1, float*& rotation2, float*& rotation3, float*& rotation4);
	void getNewCamera(float*& mat1, float*& mat2, float*& mat3, float*& mat4);

private:
	SharedMemory shared;
	char* data;
	INT64 dataSize;
};