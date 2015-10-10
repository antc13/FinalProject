#include "callbackFunctions.h"
#include "sharedMemory.h"
#include "memory.h"
#include "SharedLayouts.h"
#include "maya_includes.h"
#include <vector>

using namespace std;

Memory mem;

void transformAttributeChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData)
{
	MFnTransform transform(plug.node());

	if (msg & MNodeMessage::kAttributeSet)
	{
		TransformHeader transformHeader;
		MObject child = transform.child(0);
		MFnDagNode fnChild(child);
		transformHeader.itemNameLength = fnChild.name().length() + 1;

		MGlobal::displayInfo(transform.fullPathName());

		// Translations
		MVector translation = transform.translation(MSpace::kPostTransform);
		float translations[3];
		translations[0] = translation.x;
		translations[1] = translation.y;
		translations[2] = translation.z;

		double scale[3];

		transform.getScale(scale);
		float scaleF[3];
		scaleF[0] = scale[0];
		scaleF[1] = scale[1];
		scaleF[2] = scale[2];

		double tmp[4];
		MQuaternion quat(tmp);
		transform.getRotationQuaternion(quat.x, quat.y, quat.z, quat.w, MSpace::kPostTransform);

		float quatF[4];
		quatF[0] = quat.x;
		quatF[1] = quat.y;
		quatF[2] = quat.z;
		quatF[3] = quat.w;

		// Write to shared Mem
		char *&data = mem.getAllocatedMemory(sizeof(MessageType::mTransform) + sizeof(TransformHeader)+fnChild.name().length() + 1 + (sizeof(float)* 3) + (sizeof(float)* 3) + sizeof(float)* 4);
		MessageType type = MessageType::mTransform;
		memcpy(data, &type, sizeof(MessageType::mTransform));
		memcpy(&data[sizeof(MessageType::mTransform)], &transformHeader, sizeof(TransformHeader));

		// Name
		memcpy(&data[sizeof(MessageType::mTransform) + sizeof(TransformHeader)], fnChild.name().asChar(), fnChild.name().length() + 1);

		//// Transforms
		memcpy(&data[sizeof(MessageType::mTransform) + sizeof(TransformHeader)+fnChild.name().length() + 1], translations, sizeof(float)* 3);
		memcpy(&data[sizeof(MessageType::mTransform) + sizeof(TransformHeader)+fnChild.name().length() + 1 + (sizeof(float)* 3)], scaleF, sizeof(float)* 3);

		memcpy(&data[sizeof(MessageType::mTransform) + sizeof(TransformHeader) + fnChild.name().length() + 1 + (sizeof(float) * 3) + (sizeof(float) * 3)], quatF, sizeof(float) * 4);

		gShared.write(data, sizeof(MessageType::mTransform) + sizeof(TransformHeader)+fnChild.name().length() + 1 + (sizeof(float)* 3) + (sizeof(float)* 3) + sizeof(float)* 4);

		//MGlobal::displayInfo(MString() + "New translation: " + translation.x + " " + translation.y + " " + translation.z);
		//MGlobal::displayInfo(MString() + "New scale: " + scale[0] + " " + scale[1] + " " + scale[2]);
		//MGlobal::displayInfo(MString() + "New rotation: " + rotation.x * 180 / M_PI + " " + rotation.y * 180 / M_PI + " " + rotation.z * 180 / M_PI);
	}

}


void meshAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData)
{
	// obtain the node that contains this attribute
	MFnDagNode node(p_Plug.node());

	// Try to construct a MeshFn, if fails means the mesh is not ready
	std::string plugName(p_Plug.name().asChar());
	MStatus res;
	MFnMesh meshNode(p_Plug.node(), &res);
	if (res)
	{
		if (plugName.find("doubleSided") != std::string::npos)
		{
			// is ready!, here you shoud have access to the whole MFnMesh object to access, query, extract information from the mesh
			meshCreated(p_Plug.node());
		}
	}
}

void meshCreated(MFnMesh meshNode)
{
	MIntArray vertexList;
	MIntArray vertexCount;
	MFloatPointArray vertexPos;
	meshNode.getPoints(vertexPos, MSpace::kObject);
	meshNode.getTriangles(vertexCount, vertexList);

	vector<VertexLayout> verteciesData;
	MeshHeader meshHeader;
	meshHeader.nameLength = meshNode.name().length() + 1;
	meshHeader.vertexCount = vertexPos.length();

	VertexLayout thisVertex;
	for (unsigned int i = 0; i < vertexPos.length(); i++)
	{
		vertexPos[i].get(thisVertex.pos);
		verteciesData.push_back(thisVertex);
	}

	MIntArray triCount;
	MIntArray triVertices;
	int* triVerticesArray;
	meshNode.getTriangles(triCount, triVertices);
	meshHeader.indexCount = triVertices.length();
	triVerticesArray = new int[triVertices.length()];
	triVertices.get(triVerticesArray);

	char*& data = mem.getAllocatedMemory(sizeof(MessageType::mNewMesh) + sizeof(MeshHeader) + meshNode.name().length() + 1 + sizeof(verteciesData[0]) * verteciesData.size() + sizeof(int)* triVertices.length());

	// -- Copy message Type
	MessageType type = MessageType::mNewMesh;
	memcpy(data, &type, sizeof(MessageType::mNewMesh));
	// -- Copy mesh header;
	memcpy(&data[sizeof(MessageType::mNewMesh)], &meshHeader, sizeof(MeshHeader));
	// -- Copy name;
	memcpy(&data[sizeof(MessageType::mNewMesh) + sizeof(MeshHeader)], meshNode.name().asChar(), meshNode.name().length() + 1);
	// -- Copy vertecies;
	memcpy(&data[sizeof(MessageType::mNewMesh) + sizeof(MeshHeader) + meshNode.name().length() + 1], verteciesData.data(), sizeof(verteciesData[0]) * verteciesData.size());
	// -- Copy indecies;
	memcpy(&data[sizeof(MessageType::mNewMesh) + sizeof(MeshHeader) + meshNode.name().length() + 1 + (sizeof(verteciesData[0]) * verteciesData.size())], triVerticesArray, sizeof(int)* triVertices.length());

	gShared.write(data, sizeof(MessageType::mNewMesh) + sizeof(MeshHeader) + meshNode.name().length() + 1 + (sizeof(verteciesData[0]) * verteciesData.size()) + sizeof(int)* triVertices.length());
	delete[] triVerticesArray;

	idArray.append(MNodeMessage::addAttributeChangedCallback(meshNode.parent(0), transformAttributeChanged));
}

void cameraAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData)
{
	MFnCamera camera(p_Plug.node());
	if (p_Msg & MNodeMessage::kAttributeSet)
	{
		cameraCreated(p_Plug.node());

	}
}

void cameraCreated(MFnCamera camera)
{
	char *&data = mem.getAllocatedMemory(sizeof(MessageType::mCamera) + sizeof(float)* 4 * 4);

	MFloatMatrix projectionMatrix = camera.projectionMatrix();
	float camMatrix[4][4];
	projectionMatrix.get(camMatrix);

	MessageType type = MessageType::mCamera;
	memcpy(data, &type, sizeof(MessageType::mCamera));

	for (unsigned int i = 0; i < 4; i++)
		memcpy(&data[sizeof(MessageType::mCamera) + sizeof(float)* 4 * i], camMatrix[i], sizeof(float)* 4);
	gShared.write(data, sizeof(MessageType::mCamera) + sizeof(float)* 4 * 4);

	idArray.append(MNodeMessage::addAttributeChangedCallback(camera.parent(0), transformAttributeChanged));
}

void nodeCreated(MObject &node, void *clientData)
{
	if (node.hasFn(MFn::kMesh))
	{
		idArray.append(MNodeMessage::addAttributeChangedCallback(node, meshAttributeChanged, &clientData));
	}

	if (node.hasFn(MFn::kCamera))
	{ 
		idArray.append(MNodeMessage::addAttributeChangedCallback(node, cameraAttributeChanged, clientData));
	}
}