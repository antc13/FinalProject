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

private:
	SharedMemory shared;
	char* data;
	INT64 dataSize;
};