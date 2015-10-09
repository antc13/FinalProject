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
	void getNewTransform(char*& name, float*& translation, float*& scale, float*& rotation);
private:
	SharedMemory shared;
	char* data;
	INT64 dataSize;
};