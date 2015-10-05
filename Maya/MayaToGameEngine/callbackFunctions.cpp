#include "callbackFunctions.h"
#include "sharedMemory.h"
#include "memory.h"
#include <vector>

using namespace std;

Memory mem;

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
				MGlobal::displayInfo(MString() + "Vertex Position: " + verteciesData[i].pos[0] + " " + verteciesData[i].pos[1] + " " + verteciesData[i].pos[2]);

			}

			MIntArray triCount;
			MIntArray triVertices;
			int* triVerticesArray;
			meshNode.getTriangles(triCount, triVertices);
			meshHeader.indexCount = triVertices.length();
			triVerticesArray = new int[triVertices.length()];
			triVertices.get(triVerticesArray);

			char*& data = mem.getAllocatedMemory(sizeof(MeshHeader) + meshNode.name().length() + 1 + sizeof(verteciesData[0]) * verteciesData.size() + sizeof(int) * triVertices.length());
			
			MGlobal::displayInfo("HEADER");
			memcpy(data, &meshHeader, sizeof(MeshHeader));
			MGlobal::displayInfo("NAME");
			memcpy(&data[sizeof(MeshHeader)], meshNode.name().asChar(), meshNode.name().length() + 1);
			MGlobal::displayInfo("VERTEX");
			memcpy(&data[sizeof(MeshHeader)+meshNode.name().length() + 1], verteciesData.data(), sizeof(verteciesData[0]) * verteciesData.size());
			MGlobal::displayInfo("INDEX");
			memcpy(&data[sizeof(MeshHeader)+meshNode.name().length() + 1 + (sizeof(verteciesData[0]) * verteciesData.size())], triVerticesArray, sizeof(int)* triVertices.length());

			for (int i = 0; i < triVertices.length(); i++)
				MGlobal::displayInfo(MString() + triVerticesArray[i] + " " + triVertices[i]);
			
			gShared.Write(MessageType::mNewMesh, data, sizeof(MeshHeader)+meshNode.name().length() + 1 + (sizeof(verteciesData[0]) * verteciesData.size()) + sizeof(int)* triVertices.length());
			delete[] triVerticesArray;
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