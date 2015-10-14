#include "callbackFunctions.h"
#include "sharedMemory.h"
#include "memory.h"
#include "SharedLayouts.h"
#include "maya_includes.h"
#include <vector>
#include <list>

using namespace std;

struct MessageQueueStruct
{
	INT64 size = 0;
	char* data = nullptr;

	MessageQueueStruct(char* data, INT64 size)
	{
		this->data = new char[size];
		memcpy(this->data, data, size);
		this->size = size;
	}

	~MessageQueueStruct()
	{
		delete[] data;
	}
};

Memory mem;
list<MessageQueueStruct> messageQueue;

void transformAttributeChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData)
{
	//MFnTransform transform(plug.node());

	if (msg & MNodeMessage::kAttributeSet)
	{
		transformCreate(plug.node());
		//MGlobal::displayInfo(MString() + "New translation: " + translation.x + " " + translation.y + " " + translation.z);
		//MGlobal::displayInfo(MString() + "New scale: " + scale[0] + " " + scale[1] + " " + scale[2]);
		//MGlobal::displayInfo(MString() + "New rotation: " + rotation.x * 180 / M_PI + " " + rotation.y * 180 / M_PI + " " + rotation.z * 180 / M_PI);
	}

}
void transformCreate(MObject node)
{
	MFnTransform transform(node);
	TransformHeader transformHeader;
	MObject child = transform.child(0);
	MFnDagNode fnChild(child);
	transformHeader.itemNameLength = fnChild.name().length() + 1;

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
	char *&data = mem.getAllocatedMemory(sizeof(MessageType::mTransform) + sizeof(TransformHeader) + fnChild.name().length() + 1 + (sizeof(float) * 3) + (sizeof(float) * 3) + sizeof(float) * 4);
	MessageType type = MessageType::mTransform;
	memcpy(data, &type, sizeof(MessageType::mTransform));
	memcpy(&data[sizeof(MessageType::mTransform)], &transformHeader, sizeof(TransformHeader));

	// Name
	memcpy(&data[sizeof(MessageType::mTransform) + sizeof(TransformHeader)], fnChild.name().asChar(), fnChild.name().length() + 1);

	//// Transforms
	memcpy(&data[sizeof(MessageType::mTransform) + sizeof(TransformHeader) + fnChild.name().length() + 1], translations, sizeof(float) * 3);
	memcpy(&data[sizeof(MessageType::mTransform) + sizeof(TransformHeader) + fnChild.name().length() + 1 + (sizeof(float) * 3)], scaleF, sizeof(float) * 3);

	memcpy(&data[sizeof(MessageType::mTransform) + sizeof(TransformHeader) + fnChild.name().length() + 1 + (sizeof(float) * 3) + (sizeof(float) * 3)], quatF, sizeof(float) * 4);

	gShared.write(data, sizeof(MessageType::mTransform) + sizeof(TransformHeader) + fnChild.name().length() + 1 + (sizeof(float) * 3) + (sizeof(float) * 3) + sizeof(float) * 4);

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
		else if (p_Msg & MNodeMessage::AttributeMessage::kAttributeEval)
		{
			meshVertecChanged(p_Plug.node());
		}
	}
}

void meshCreated(MObject node)
{
	MFnMesh meshNode(node);
	MFloatPointArray vertexPos;
	meshNode.getPoints(vertexPos, MSpace::kObject);

	MIntArray triCount;
	MIntArray triVertices;
	int* triVerticesArray;
	meshNode.getTriangles(triCount, triVertices);
	triVerticesArray = new int[triVertices.length()];
	triVertices.get(triVerticesArray);

	vector<VertexLayout> verteciesData;

	VertexLayout thisVertex;
	MVector normal;
	for (unsigned int i = 0; i < vertexPos.length(); i++)
	{
		meshNode.getVertexNormal(i, true, normal, MSpace::kObject);
		vertexPos[i].get(thisVertex.pos);
		thisVertex.normal[0] = normal.x;
		thisVertex.normal[1] = normal.y;
		thisVertex.normal[2] = normal.z;
		verteciesData.push_back(thisVertex);
	}

	MeshHeader meshHeader;
	meshHeader.nameLength = meshNode.name().length() + 1;
	meshHeader.vertexCount = verteciesData.size();
	meshHeader.indexCount = triVertices.length();

	char*& data = mem.getAllocatedMemory(sizeof(MessageType::mNewMesh) + sizeof(MeshHeader) + meshNode.name().length() + 1 + sizeof(verteciesData[0]) * verteciesData.size() + sizeof(int)* triVertices.length());

	// -- Copy message Type
	MessageType type = MessageType::mNewMesh;

	UINT64 offset = 0;

	memcpy(&data[offset], &type, sizeof(MessageType::mNewMesh));
	offset += sizeof(MessageType::mNewMesh);
	// -- Copy mesh header;
	memcpy(&data[offset], &meshHeader, sizeof(MeshHeader));
	offset += sizeof(MeshHeader);
	// -- Copy name;
	memcpy(&data[offset], meshNode.name().asChar(), meshNode.name().length() + 1);
	offset += meshNode.name().length() + 1;
	// -- Copy vertecies;
	memcpy(&data[offset], verteciesData.data(), sizeof(verteciesData[0]) * verteciesData.size());
	offset += sizeof(verteciesData[0]) * verteciesData.size();
	// -- Copy indecies;
	memcpy(&data[offset], triVerticesArray, sizeof(int)* triVertices.length());
	offset += sizeof(int)* triVertices.length();
	delete[] triVerticesArray;

	if (messageQueue.size() == 0)
		gShared.write(data, offset);
	else
	{
		MessageQueueStruct queueData(data, offset);
		messageQueue.push_back(queueData);
	}

	idArray.append(MNodeMessage::addAttributeChangedCallback(meshNode.parent(0), transformAttributeChanged));
	idArray.append(MNodeMessage::addNodePreRemovalCallback(node, nodeRemoval));
}

void meshVertecChanged(MObject node)
{
	MFnMesh meshNode(node);
	MPointArray verticesPos;
	meshNode.getPoints(verticesPos, MSpace::kObject);
	MRichSelection activeList;
	MGlobal::getRichSelection(activeList, true);
	MSelectionList list1;
	MSelectionList list2;
	activeList.getSelection(list1);
	activeList.getSymmetry(list2);
	list1.merge(list2, MSelectionList::kMergeNormal);
	MItSelectionList iter(list1);
	
	vector<UINT>vertexIDsAlreadySent;

	for (; !iter.isDone(); iter.next())
	{
		MDagPath item;
		MObject component;
		iter.getDagPath(item, component);
		if (item.node() == node)
		{
			if (component.apiType() == MFn::kMeshVertComponent)
			{
				MFnSingleIndexedComponent vertices(component);
				MIntArray vertexIDs;
				vertices.getElements(vertexIDs);
				vertexIDsAlreadySent.resize(vertexIDs.length());
				for (unsigned int i = 0; i < vertexIDs.length(); i++)
					vertexIDsAlreadySent[i] = vertexIDs[i];
			}
			else if (component.apiType() == MFn::kMeshEdgeComponent)
			{
				MFnSingleIndexedComponent edges(component);
				MIntArray edgeIDs;
				edges.getElements(edgeIDs);
				for (unsigned int i = 0; i < edgeIDs.length(); i++)
				{
					int2 vertexIDs;
					meshNode.getEdgeVertices(edgeIDs[i], vertexIDs);
					for (unsigned int i = 0; i < 2; i++)
					{
						if (!(find(vertexIDsAlreadySent.begin(), vertexIDsAlreadySent.end(), vertexIDs[i]) != vertexIDsAlreadySent.end()))
						{
							vertexIDsAlreadySent.push_back(vertexIDs[i]);
						}
					}
				}
			}
			else if (component.apiType() == MFn::kMeshPolygonComponent)
			{
				MFnSingleIndexedComponent faces(component);
				MIntArray faceIDs;
				faces.getElements(faceIDs);
				for (unsigned int i = 0; i < faceIDs.length(); i++)
				{
					MIntArray vertexIDs;
					meshNode.getPolygonVertices(faceIDs[i], vertexIDs);
					for (unsigned int i = 0; i < vertexIDs.length(); i++)
					{
						if (!(find(vertexIDsAlreadySent.begin(), vertexIDsAlreadySent.end(), vertexIDs[i]) != vertexIDsAlreadySent.end()))
						{
							vertexIDsAlreadySent.push_back(vertexIDs[i]);
						}
					}
				}
			}
			//------ Send the Vertecies Changed ---------
			MFloatPointArray vertexPos;
			meshNode.getPoints(vertexPos, MSpace::kObject);
			vector<VertexLayout>verteciesData;
			VertexLayout thisVertex;
			MVector normal;
			for (unsigned int i = 0; i < vertexIDsAlreadySent.size(); i++)
			{
				meshNode.getVertexNormal(vertexIDsAlreadySent[i], true, normal, MSpace::kObject);
				vertexPos[vertexIDsAlreadySent[i]].get(thisVertex.pos);
				thisVertex.normal[0] = normal.x;
				thisVertex.normal[1] = normal.y;
				thisVertex.normal[2] = normal.z;
				verteciesData.push_back(thisVertex);
			}

			VertexChangeHeader header;
			header.nameLength = meshNode.name().length() + 1;
			header.numVerteciesChanged = vertexIDsAlreadySent.size();

			char* data = mem.getAllocatedMemory(sizeof(MessageType::mVertexChange) + sizeof(VertexChangeHeader) + meshNode.name().length() + 1 + vertexIDsAlreadySent.size() * sizeof(vertexIDsAlreadySent[0]) + verteciesData.size() * sizeof(verteciesData[0]));

			UINT64 offset = 0;
			MessageType type = MessageType::mVertexChange;

			memcpy(&data[offset], &type, sizeof(MessageType::mVertexChange));
			offset += sizeof(MessageType::mVertexChange);

			memcpy(&data[offset], &header, sizeof(VertexChangeHeader));
			offset += sizeof(VertexChangeHeader);

			memcpy(&data[offset], meshNode.name().asChar(), meshNode.name().length() + 1);
			offset += meshNode.name().length() + 1;

			memcpy(&data[offset], verteciesData.data(), verteciesData.size() * sizeof(verteciesData[0]));
			offset += verteciesData.size() * sizeof(verteciesData[0]);

			memcpy(&data[offset], vertexIDsAlreadySent.data(), vertexIDsAlreadySent.size() * sizeof(vertexIDsAlreadySent[0]));
			offset += vertexIDsAlreadySent.size() * sizeof(vertexIDsAlreadySent[0]);

			if (messageQueue.size() == 0)
				gShared.write(data, offset);
			else
			{
				MessageQueueStruct queueData(data, offset);
				messageQueue.push_back(queueData);
			}

			break;
		}
	}
}

void cameraChanged(const MString &str, MObject &node, void *clientData)
{
	MFnDagNode thisNode(node);
	cameraCreated(node);
	transformCreate(thisNode.parent(0));
	MGlobal::displayInfo("HEJ");
}

void cameraAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData)
{
	MFnCamera camera(p_Plug.node());
	if (p_Msg & MNodeMessage::kAttributeSet)
	{
		cameraCreated(p_Plug.node());
	}
}

void cameraCreated(MObject node)
{
	MFnCamera camera(node);
	NodeRemovedHeader camHeader;
	camHeader.nameLength = camera.name().length() + 1;

	bool isOrtho;
	isOrtho = camera.isOrtho();
	char *&data = mem.getAllocatedMemory(sizeof(MessageType::mCamera) + sizeof(NodeRemovedHeader) + camera.name().length() + 1 + (sizeof(float)* 4 * 4) + sizeof(isOrtho));

	MFloatMatrix projectionMatrix = camera.projectionMatrix();
	float camMatrix[4][4];
	projectionMatrix.get(camMatrix);

	MessageType type = MessageType::mCamera;
	UINT64 offset = 0;

	memcpy(data, &type, sizeof(MessageType::mCamera));
	offset += sizeof(MessageType::mCamera);

	memcpy(&data[offset], &camHeader, sizeof(NodeRemovedHeader));
	offset += sizeof(NodeRemovedHeader);

	memcpy(&data[offset], camera.name().asChar(), camHeader.nameLength);
	offset += camHeader.nameLength;

	for (unsigned int i = 0; i < 4; i++)
		memcpy(&data[offset + sizeof(float)* 4 * i], camMatrix[i], sizeof(float)* 4);

	offset += sizeof(float)* 4 * 4;

	memcpy(&data[offset], &isOrtho, sizeof(isOrtho));
	offset += sizeof(isOrtho);

	gShared.write(data, offset);

	if (messageQueue.size() == 0)
		gShared.write(data, sizeof(MessageType::mCamera) + sizeof(float) * 4 * 4);
	else
	{
		MessageQueueStruct queueData(data, sizeof(MessageType::mCamera) + sizeof(float) * 4 * 4);
		messageQueue.push_back(queueData);
	}

	idArray.append(MNodeMessage::addAttributeChangedCallback(camera.parent(0), transformAttributeChanged));
	idArray.append(MNodeMessage::addNodePreRemovalCallback(node, nodeRemoval));
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

void nodeRemoval(MObject &node, void *clientData)
{
	if (node.hasFn(MFn::kMesh) || node.hasFn(MFn::kCamera))
	{
		MCallbackIdArray ids;
		MMessage::nodeCallbacks(node, ids);
		MMessage::removeCallbacks(ids);

		NodeRemovedHeader header;
		MFnDependencyNode tmp(node);
		header.nameLength = tmp.name().length() + 1;

		MessageType type = MessageType::mNodeRemoved;
		char* data = mem.getAllocatedMemory(sizeof(MessageType::mNodeRemoved) + sizeof(NodeRemovedHeader) + header.nameLength);

		UINT64 offset = 0;

		memcpy(&data[offset], &type, sizeof(MessageType::mNodeRemoved));
		offset += sizeof(MessageType::mNodeRemoved);

		memcpy(&data[offset], &header, sizeof(NodeRemovedHeader));
		offset += sizeof(NodeRemovedHeader);

		memcpy(&data[offset], tmp.name().asChar(), header.nameLength);
		offset += header.nameLength;

		if (messageQueue.size() == 0)
			gShared.write(data, offset);
		else
		{
			MessageQueueStruct queueData(data, offset);
			messageQueue.push_back(queueData);
		}
	}
	else if (node.hasFn(MFn::kTransform))
	{
		MCallbackIdArray ids;
		MMessage::nodeCallbacks(node, ids);
		MMessage::removeCallbacks(ids);
	}
}

void timer(float elapsedTime, float lastTime, void *clientData)
{
	if (messageQueue.size() > 0)
		while (gShared.write(messageQueue.front().data, messageQueue.front().size))
			messageQueue.pop_front();
}