#include "MayaData.h"

MayaData::MayaData(){
	shared.initialize(200 * 1024 * 1024, (LPCWSTR)"MayaToGameEngine", false);
	data = nullptr;
	dataSize = 0;
}

MayaData::~MayaData(){

}

MessageType MayaData::read(){
	if (shared.read(data, dataSize))
		return *(MessageType*)data;
	return MessageType::mNoMessage;
}

void MayaData::getNewMesh(char*& name, VertexLayout*& vertecies, UINT& numVertecies, UINT*& indecies, UINT& numIndecies){

	MeshHeader* header = (MeshHeader*)&data[sizeof(MessageType::mNewMesh)];
	name = new char[header->nameLength];
	memcpy(name, &data[sizeof(MessageType::mNewMesh) + sizeof(MeshHeader)], header->nameLength);

	numVertecies = header->vertexCount;
	vertecies = new VertexLayout[header->vertexCount];
	memcpy(vertecies, &data[sizeof(MessageType::mNewMesh) + sizeof(MeshHeader) + header->nameLength], (header->vertexCount * sizeof(VertexLayout)));

	numIndecies = header->indexCount;
	indecies = new UINT[header->indexCount];
	memcpy(indecies, &data[sizeof(MessageType::mNewMesh) + sizeof(MeshHeader) + header->nameLength + (header->vertexCount * sizeof(VertexLayout))], header->indexCount * sizeof(UINT));
}

void MayaData::getNewTransform(char*& name, float translation[3], float scale[3], float rotation[4])
{
	TransformHeader* header = (TransformHeader*)&data[sizeof(MessageType::mTransform)];
	name = new char[header->itemNameLength];
	memcpy(name, &data[sizeof(MessageType::mTransform) + sizeof(TransformHeader)], header->itemNameLength);

	memcpy(translation, &data[sizeof(MessageType::mTransform) + sizeof(TransformHeader)+header->itemNameLength], sizeof(float) * 3);
	
	memcpy(scale, &data[sizeof(MessageType::mTransform) + sizeof(TransformHeader)+header->itemNameLength + sizeof(float) * 3], sizeof(float)* 3);
	
	memcpy(rotation, &data[sizeof(MessageType::mTransform) + sizeof(TransformHeader)+header->itemNameLength + sizeof(float)* 3 + sizeof(float) * 3], sizeof(float) * 4);
}

void MayaData::getNewCamera(float*& mat1, float*& mat2, float*& mat3, float*& mat4)
{
	mat1 = new float;
	mat2 = new float;
	mat3 = new float;
	mat4 = new float;

	memcpy(mat1, &data[sizeof(MessageType::mCamera)], sizeof(float) * 4);
	memcpy(mat2, &data[sizeof(MessageType::mCamera) + sizeof(float) * 4], sizeof(float)* 4);
	memcpy(mat3, &data[sizeof(MessageType::mCamera) + sizeof(float) * 4 * 2], sizeof(float)* 4);
	memcpy(mat4, &data[sizeof(MessageType::mCamera) + sizeof(float) * 4 * 3], sizeof(float)* 4);
}

void MayaData::getVertexChanged(char*& name, VertexLayout*& verteciesData, UINT*& indexNumbers, UINT& numVerteciesChanged)
{
	VertexChangeHeader* header = (VertexChangeHeader*)&data[sizeof(MessageType::mVertexChange)];
	UINT64 offset = sizeof(MessageType::mVertexChange) + sizeof(VertexChangeHeader);

	name = new char[header->nameLength];
	memcpy(name, &data[offset], header->nameLength);
	offset += header->nameLength;

	numVerteciesChanged = header->numVerteciesChanged;

	verteciesData = new VertexLayout[header->numVerteciesChanged];
	memcpy(verteciesData, &data[offset], header->numVerteciesChanged * sizeof(VertexLayout));
	offset += header->numVerteciesChanged * sizeof(VertexLayout);

	indexNumbers = new UINT[header->numVerteciesChanged];
	memcpy(indexNumbers, &data[offset], header->numVerteciesChanged * sizeof(UINT));
}

void MayaData::getRemoveNode(char*& name)
{
	NodeRemovedHeader* header = (NodeRemovedHeader*)&data[sizeof(MessageType::mNodeRemoved)];
	UINT64 offset = sizeof(MessageType::mNodeRemoved) + sizeof(NodeRemovedHeader);

	name = new char[header->nameLength];
	memcpy(name, &data[offset], header->nameLength);
}