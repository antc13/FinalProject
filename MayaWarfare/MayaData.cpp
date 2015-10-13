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

void MayaData::getNewCamera(char*& name, float camMatrix[4][4], bool* isOrtho)
{
	NodeRemovedHeader* header = (NodeRemovedHeader*)&data[sizeof(MessageType::mCamera)];
	name = new char[header->nameLength];
	UINT64 offset = sizeof(MessageType::mCamera) + sizeof(NodeRemovedHeader);
	memcpy(name, &data[offset], header->nameLength);

	offset += header->nameLength;

	memcpy(camMatrix, &data[offset], sizeof(float) * 4 * 4);

	offset += sizeof(float) * 4 * 4;

	memcpy(isOrtho, &data[offset], sizeof(bool));
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