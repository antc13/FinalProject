#include "MayaData.h"

MayaData::MayaData(){
	shared.initialize(200 * 1024 * 1024, (LPCWSTR)"MayaToGameEngine", false);
	data = nullptr;
	dataSize = 0;
}

MayaData::~MayaData(){
	delete[] data;
}

MessageType MayaData::read(){
	if (shared.read(data, dataSize))
		return *(MessageType*)data;
	return MessageType::mNoMessage;
}

void MayaData::getNewMesh(unsigned char UUID[UUIDSIZE], char*& name, VertexLayout*& vertecies, UINT& numVertecies, UINT*& indecies, UINT& numIndecies){

	memcpy(UUID, &data[sizeof(MessageType::mNewMesh)], UUIDSIZE);

	MeshHeader* header = (MeshHeader*)&data[sizeof(MessageType::mNewMesh) + UUIDSIZE];
	name = new char[header->nameLength];
	memcpy(name, &data[sizeof(MessageType::mNewMesh) + UUIDSIZE + sizeof(MeshHeader)], header->nameLength);

	numVertecies = header->vertexCount;
	vertecies = new VertexLayout[header->vertexCount];
	memcpy(vertecies, &data[sizeof(MessageType::mNewMesh) + UUIDSIZE + sizeof(MeshHeader) + header->nameLength], (header->vertexCount * sizeof(VertexLayout)));

	numIndecies = header->indexCount;
	indecies = new UINT[header->indexCount];
	memcpy(indecies, &data[sizeof(MessageType::mNewMesh) + UUIDSIZE + sizeof(MeshHeader) + header->nameLength + (header->vertexCount * sizeof(VertexLayout))], header->indexCount * sizeof(UINT));
}

void MayaData::getNewTransform(unsigned char UUID[UUIDSIZE], unsigned char shapeUUID[UUIDSIZE], ShapeType& shapeType, char*& name, char*& parentName, float translation[3], float scale[3], float rotation[4])
{
	UINT64 offset = sizeof(MessageType::mTransform);

	memcpy(UUID, &data[offset], UUIDSIZE);
	offset += UUIDSIZE;

	TransformHeader* header = (TransformHeader*)&data[offset];
	offset += sizeof(TransformHeader);

	memcpy(&shapeType, &header->shape, sizeof(ShapeType));

	memcpy(shapeUUID, &data[offset], UUIDSIZE);
	offset += UUIDSIZE;

	name = new char[header->itemNameLength];
	memcpy(name, &data[offset], header->itemNameLength);
	offset += header->itemNameLength;

	if (header->parentNameLength > 0)
	{
		parentName = new char[header->parentNameLength];
		memcpy(parentName, &data[offset], header->parentNameLength);
		offset += header->parentNameLength;
	}
	else
		parentName = nullptr;

	memcpy(translation, &data[offset], sizeof(float) * 3);
	offset += sizeof(float) * 3;

	memcpy(scale, &data[offset], sizeof(float) * 3);
	offset += sizeof(float) * 3;

	memcpy(rotation, &data[offset], sizeof(float) * 4);
}

void MayaData::getNewCamera(unsigned char UUID[UUIDSIZE], char*& name, float camMatrix[4][4], bool* isOrtho)
{
	memcpy(UUID, &data[sizeof(MessageType::mCamera)], UUIDSIZE);

	NodeRemovedHeader* header = (NodeRemovedHeader*)&data[sizeof(MessageType::mCamera) + UUIDSIZE];
	name = new char[header->nameLength];
	UINT64 offset = sizeof(MessageType::mCamera) + UUIDSIZE + sizeof(NodeRemovedHeader);
	memcpy(name, &data[offset], header->nameLength);

	offset += header->nameLength;

	memcpy(camMatrix, &data[offset], sizeof(float) * 4 * 4);

	offset += sizeof(float) * 4 * 4;

	memcpy(isOrtho, &data[offset], sizeof(bool));
}

void MayaData::getVertexChanged(unsigned char UUID[UUIDSIZE], VertexLayout*& verteciesData, UINT*& indexNumbers, UINT& numVerteciesChanged)
{
	memcpy(UUID, &data[sizeof(MessageType::mVertexChange)], UUIDSIZE);
	VertexChangeHeader* header = (VertexChangeHeader*)&data[sizeof(MessageType::mVertexChange)];
	UINT64 offset = sizeof(MessageType::mVertexChange) + sizeof(VertexChangeHeader);

	VertexChangeHeader* header = (VertexChangeHeader*)&data[sizeof(MessageType::mVertexChange) + UUIDSIZE];
	UINT64 offset = sizeof(MessageType::mVertexChange) + UUIDSIZE + sizeof(VertexChangeHeader);
	name = new char[header->nameLength];
	memcpy(name, &data[offset], header->nameLength);
	offset += header->nameLength;

	numVerteciesChanged = header->numVerteciesChanged;

	verteciesData = new VertexLayout[header->numVerteciesChanged];
	memcpy(verteciesData, &data[offset], header->numVerteciesChanged * sizeof(VertexLayout));
	offset += header->numVerteciesChanged * sizeof(VertexLayout);

	//indexNumbers = new UINT[header->numVerteciesChanged];
	//memcpy(indexNumbers, &data[offset], header->numVerteciesChanged * sizeof(UINT));
}

void MayaData::getRemoveNode(unsigned char UUID[UUIDSIZE], char*& name)
{
	color[0] = 0;
	color[1] = 0;
	color[2] = 0;
	range = 0;

	UINT64 offset = sizeof(MessageType::mLight);
	memcpy(color, &data[offset], sizeof(float) * 3);

	offset += sizeof(float) * 3;

	memcpy(&range, &data[offset], sizeof(float));

}

void MayaData::getRemoveNode(char*& name)
{
	memcpy(UUID, &data[sizeof(MessageType::mNodeRemoved)], UUIDSIZE);

	NodeRemovedHeader* header = (NodeRemovedHeader*)&data[sizeof(MessageType::mNodeRemoved) + UUIDSIZE];
	UINT64 offset = sizeof(MessageType::mNodeRemoved) + UUIDSIZE + sizeof(NodeRemovedHeader);

	name = new char[header->nameLength];
	memcpy(name, &data[offset], header->nameLength);
}