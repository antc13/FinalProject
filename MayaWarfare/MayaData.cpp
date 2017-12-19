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

void MayaData::getNewCamera(char*& name, float camMatrix[4][4])
{
	NodeRemovedHeader* header = (NodeRemovedHeader*)&data[sizeof(MessageType::mCamera)];
	name = new char[header->nameLength];
	UINT64 offset = sizeof(MessageType::mCamera) + sizeof(NodeRemovedHeader);
	memcpy(name, &data[offset], header->nameLength);

	offset += header->nameLength;

	memcpy(camMatrix, &data[offset], sizeof(float) * 4 * 4);

	offset += sizeof(float) * 4 * 4;
}

void MayaData::getCameraChanged(char*& name)
{
	NameHeader* header = (NameHeader*)&data[sizeof(MessageType::mCameraChanged)];
	name = new char[header->nameLength];

	UINT64 offset = sizeof(MessageType::mCameraChanged) + sizeof(NameHeader);
	memcpy(name, &data[offset], header->nameLength);

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

	//indexNumbers = new UINT[header->numVerteciesChanged];
	//memcpy(indexNumbers, &data[offset], header->numVerteciesChanged * sizeof(UINT));
}

void MayaData::getLight(float color[3], float& range)
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

void MayaData::getMaterial(char*& name, char*& texturePath, float diffuseColor[3], float ambientColor[3])
{

	diffuseColor[0] = 0;
	diffuseColor[1] = 0;
	diffuseColor[2] = 0;
	MaterialHeader* header = (MaterialHeader*)&data[sizeof(MessageType::mNewMaterial)];
	UINT64 offset = sizeof(MessageType::mNewMaterial) + sizeof(MaterialHeader);

	name = new char[header->materialNameLength];
	memcpy(name, &data[offset], header->materialNameLength);

	offset += header->materialNameLength;

	if (header->texturePathLength > 0)
	{
		texturePath = new char[header->texturePathLength];
		memcpy(texturePath, &data[offset], header->texturePathLength);
		offset += header->texturePathLength;
	}
	else
		texturePath = nullptr;
	memcpy(ambientColor, &data[offset], sizeof(float) * 3);
	offset += sizeof(float) * 3;

	memcpy(diffuseColor, &data[offset], sizeof(float) * 3);
	
}

void MayaData::getMeshMaterialNames(char*& meshName, char*& materialName)
{
	MeshMaterialNamesHeader* header = (MeshMaterialNamesHeader*)&data[sizeof(MessageType::mMeshChangedMaterial)];
	UINT64 offset = sizeof(MessageType::mMeshChangedMaterial) + sizeof(MeshMaterialNamesHeader);

	meshName = new char[header->meshNameLength];
	memcpy(meshName, &data[offset], header->meshNameLength);

	offset += header->meshNameLength;

	materialName = new char[header->materialNameLength];
	memcpy(materialName, &data[offset], header->materialNameLength);
}

void MayaData::getRemoveNode(char*& name)
{
	NodeRemovedHeader* header = (NodeRemovedHeader*)&data[sizeof(MessageType::mNodeRemoved)];
	UINT64 offset = sizeof(MessageType::mNodeRemoved) + sizeof(NodeRemovedHeader);

	name = new char[header->nameLength];
	memcpy(name, &data[offset], header->nameLength);
}