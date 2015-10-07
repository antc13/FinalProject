#include "sharedMemory.h"
#include "Memory.h"
#include <iostream>
#include <vector>

using namespace std;
SharedMemory gShared;
Memory mem;
int main()
{
	gShared.initialize(200 * 1024 * 1024, (LPCWSTR)"MayaToGameEngine", false);
	void* data = nullptr;
	MessageType type;
	size_t length;
	while (true)
	{
		char* data = mem.getAllocatedMemory(0);
		type = gShared.Read(data, mem.getSize(), length);
		if (type)
		{
			int test = sizeof(MeshHeader);
			//((char*)data)[length] = 0;
			if (type == MessageType::mNewMesh)
			{
				MeshHeader* header = (MeshHeader*)data;
				VertexLayout* verteciesData;
				verteciesData = (VertexLayout*)&((char*)data)[sizeof(MeshHeader)];
				cout << "New Mesh" << endl;
				cout << "NameLength: " << header->nameLength << endl;
				cout << "NumVertecies: " << header->vertexCount << endl;
				cout << "NumIndecies: " << header->indexCount << endl;
				cout << "Data: " << endl;
				for (size_t i = 0; i < header->vertexCount; i++)
					cout << verteciesData[i].pos[0] << " " << verteciesData[i].pos[1] << " " << verteciesData[i].pos[2] << endl;
				int* index = (int*)&((char*)data)[sizeof(MeshHeader)+(header->vertexCount * sizeof(VertexLayout))];
				for (size_t i = 0; i < header->indexCount; i++)
					cout << index[i] << endl;

			}
		}
	}
}