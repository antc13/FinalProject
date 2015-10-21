#pragma once

enum MessageType{ mNoMessage = 0, mNewMesh, mLight, mCamera, mTransform, mNewMaterial, mMeshChangedMaterial, mVertexChange, mNameChange, mNodeRemoved};

struct MeshHeader
{
	INT nameLength;
	INT vertexCount;
	INT indexCount;
};

struct VertexChangeHeader
{
	INT nameLength;
	INT numVerteciesChanged;
};

struct VertexLayout
{
	float pos[3];
	float normal[3];
};

struct NodeRemovedHeader
{
	INT nameLength;
};

struct MeshMaterialNamesHeader
{
	INT meshNameLength;
	INT materialNameLength;
};