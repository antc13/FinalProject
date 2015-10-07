#include "callbackFunctions.h"
#include "sharedMemory.h"
#include "memory.h"
#include "SharedLayouts.h"
#include <vector>

using namespace std;

Memory mem;

void transformAttributeChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData)
{
	MFnTransform transform(plug.node());

	if (msg & MNodeMessage::kAttributeSet)
	{
		TransformHeader transformHeader;
		transformHeader.itemNameLength = transform.name().length() + 1;

		MGlobal::displayInfo(transform.fullPathName());

		// Translations
		MVector translation = transform.translation(MSpace::kObject);
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

		MEulerRotation rotation;
		transform.getRotation(rotation);
		MMatrix rotationMatrix = rotation.asMatrix();
		float rotFloatMatrix[4][4];
		rotationMatrix.get(rotFloatMatrix);

		// Write to shared Mem
		char *&data = mem.getAllocatedMemory(sizeof(TransformHeader)+transform.name().length() + 1 + (sizeof(float)* 3) + (sizeof(float)* 3) + sizeof(float)* 4 * 4);
		memcpy(data, &transformHeader, sizeof(TransformHeader));

		// Name
		memcpy(&data[sizeof(TransformHeader)], transform.name().asChar(), transform.name().length() + 1);

		//// Transforms
		memcpy(&data[sizeof(TransformHeader)+transform.name().length() + 1], translations, sizeof(float)* 3);
		memcpy(&data[sizeof(TransformHeader)+transform.name().length() + 1 + (sizeof(float)* 3)], scale, sizeof(float)* 3);
		memcpy(&data[sizeof(TransformHeader)+transform.name().length() + 1 + (sizeof(float)* 3) + (sizeof(float)* 3)], rotFloatMatrix, sizeof(float)* 4 * 4);

		gShared.Write(MessageType::mTransform, data, sizeof(TransformHeader)+transform.name().length() + 1 + (sizeof(float)* 3) + (sizeof(float)* 3) + sizeof(float)* 4 * 4);

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

	if (plugName.find("doubleSided") != std::string::npos && MNodeMessage::AttributeMessage::kAttributeSet)
	{
		if (res)
		{
			// is ready!, here you shoud have access to the whole MFnMesh object to access, query, extract information from the mesh

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
			memcpy(&data[sizeof(MessageType::mNewMesh) + sizeof(MeshHeader)+meshNode.name().length() + 1], verteciesData.data(), sizeof(verteciesData[0]) * verteciesData.size());
			// -- Copy indecies;
			memcpy(&data[sizeof(MessageType::mNewMesh) + sizeof(MeshHeader)+meshNode.name().length() + 1 + (sizeof(verteciesData[0]) * verteciesData.size())], triVerticesArray, sizeof(int)* triVertices.length());
			
			gShared.write(data, sizeof(MessageType::mNewMesh) + sizeof(MeshHeader)+meshNode.name().length() + 1 + (sizeof(verteciesData[0]) * verteciesData.size()) + sizeof(int)* triVertices.length());
			delete[] triVerticesArray;

			idArray.append(MNodeMessage::addAttributeChangedCallback(meshNode.parent(0), transformAttributeChanged));
		}
	}
}

void nodeCreated(MObject &node, void *clientData)
{
	if (node.hasFn(MFn::kDependencyNode))
	{

		MFnDependencyNode nodeFn(node);

		MGlobal::displayInfo(MString() + "Created Node: " + nodeFn.name());
	}

	if (node.hasFn(MFn::kMesh))
	{
		idArray.append(MNodeMessage::addAttributeChangedCallback(node, meshAttributeChanged));
	}
}