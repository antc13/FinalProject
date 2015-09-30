#include "callbackFunctions.h"
#include "sharedMemory.h"

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
			MPointArray vertexPos;
			meshNode.getPoints(vertexPos, MSpace::kObject);
			meshNode.getTriangles(vertexCount, vertexList);

			double points[4];
			for (unsigned int i = 0; i < vertexPos.length(); i++)
			{
				vertexPos[i].get(points);
				MGlobal::displayInfo(MString() + "Vertex Position: " + points[0] + " " + points[1] + " " + points[2]);
			}
			MeshHeader meshHeader;
			meshHeader.nameLength = meshNode.name().length();
			meshHeader.vertexCount = vertexPos.length();
			const float* pts = meshNode.getRawPoints(&res);
			char* data = new char[sizeof(MeshHeader) + vertexPos.length() * 3];
			memcpy(data, &meshHeader, sizeof(MeshHeader));
			memcpy(&data[sizeof(MeshHeader)], pts, +vertexPos.length() * 3);

			gShared.Write(MessageType::mNewMesh, data, sizeof(MeshHeader) + vertexPos.length() * 3);
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