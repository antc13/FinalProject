#pragma once

enum MessageType{ mNoMessage = 0, mNewMesh, mLight, mCamera, mTransform, mMaterial, mVertexChange, mNameChange };

struct MeshHeader
{
	INT64 nameLength;
	INT64 vertexCount;
	INT64 indexCount;
};

struct VertexLayout
{
	float pos[3];
};