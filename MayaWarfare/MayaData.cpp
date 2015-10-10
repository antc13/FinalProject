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

void MayaData::getNewMesh(char*& name, VertexLayout*& vertecies, UINT64& numVertecies, UINT*& indecies, UINT64& numIndecies){

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

void MayaData::getNewTransform(char*& name, float*& translation, float*& scale, float*& rotation)
{
	TransformHeader* header = (TransformHeader*)&data[sizeof(MessageType::mTransform)];
	name = new char[header->itemNameLength];
	memcpy(name, &data[sizeof(MessageType::mTransform) + sizeof(TransformHeader)], header->itemNameLength);

	translation = new float;
	memcpy(translation, &data[sizeof(MessageType::mTransform) + sizeof(TransformHeader)+header->itemNameLength], sizeof(float) * 3);
	scale = new float;
	memcpy(scale, &data[sizeof(MessageType::mTransform) + sizeof(TransformHeader)+header->itemNameLength + sizeof(float) * 3], sizeof(float)* 3);
	
	rotation = new float[4];

	memcpy(rotation, &data[sizeof(MessageType::mTransform) + sizeof(TransformHeader)+header->itemNameLength + sizeof(float)* 3 + sizeof(float) * 3], sizeof(float) * 4);
}