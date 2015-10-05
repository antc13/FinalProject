#include "sharedMemory.h"
#include <iostream>
#include <vector>

using namespace std;
SharedMemory gShared;

int main()
{
	gShared.initialize(200 * 1024 * 1024, (LPCWSTR)"MayaToGameEngine", false);
	void* data = nullptr;
	MessageType type;
	size_t length;
	while (true)
	{
		type = gShared.Read(data, length);
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
				cout << "Data: " << endl;
				for (size_t i = 0; i < header->vertexCount; i++)
					cout << verteciesData[i].pos[0] << " " << verteciesData[i].pos[1] << " " << verteciesData[i].pos[2] << endl;
			}
		}
	}
}