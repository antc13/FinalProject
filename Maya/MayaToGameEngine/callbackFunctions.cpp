#include "callbackFunctions.h"
#include "sharedMemory.h"
#include "memory.h"
#include "SharedLayouts.h"
#include "maya_includes.h"
#include <vector>
#include <list>
#include <map>

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
void transformCreate(MObject &node)
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
	transform.getRotationQuaternion(quat.x, quat.y, quat.z, quat.w, MSpace::kObject);

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

void meshy(MObject& node)
{
	MPlugArray plugArray;
	MFnMesh meshNode(node);
	MObjectArray objectArray;
	MIntArray dummyArray;
	meshNode.getConnectedShaders(0, objectArray, dummyArray);
	if (objectArray.length() > 0)
	{
		MFnDependencyNode shader(objectArray[0]);
		MPlug p_Plug = shader.findPlug("surfaceShader");
		if (p_Plug.connectedTo(plugArray, true, false))
		{
			MFnDependencyNode depNode(plugArray[0].node());

			MeshMaterialNamesHeader header;
			header.meshNameLength = meshNode.name().length() + 1;
			header.materialNameLength = depNode.name().length() + 1;

			UINT64 offset = 0;

			char *&data = mem.getAllocatedMemory(sizeof(MessageType::mMeshChangedMaterial) + sizeof(MeshMaterialNamesHeader)+header.meshNameLength + header.materialNameLength);
			MessageType type = MessageType::mMeshChangedMaterial;

			memcpy(data, &type, sizeof(MessageType::mMeshChangedMaterial));
			offset += sizeof(MessageType::mMeshChangedMaterial);

			memcpy(&data[offset], &header, sizeof(header));
			offset += sizeof(header);

			memcpy(&data[offset], meshNode.name().asChar(), header.meshNameLength);
			offset += header.meshNameLength;

			memcpy(&data[offset], depNode.name().asChar(), header.materialNameLength);
			offset += header.materialNameLength;

			if (messageQueue.size() == 0)
			{
				if (gShared.write(data, offset))
				{
					return;
				}
			}
			MessageQueueStruct queueData(data, offset);
			messageQueue.push_back(queueData);
		}
	}
}

void meshAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData)
{
	// obtain the node that contains this attribute
	MFnDagNode node(p_Plug.node());

	// Try to construct a MeshFn, if fails means the mesh is not ready
	std::string plugName(p_Plug.name().asChar());
	MGlobal::displayInfo(p_Plug.name());
	MStatus res;
	MFnMesh meshNode(p_Plug.node(), &res);
	if (res)
	{
		if (plugName.find("doubleSided") != std::string::npos)
		{
			// is ready!, here you shoud have access to the whole MFnMesh object to access, query, extract information from the mesh
			MGlobal::displayInfo("New Mesh");
			MGlobal::displayInfo(node.name());
			meshCreated(p_Plug.node());
			meshy(p_Plug.node());
			idArray.append(MNodeMessage::addAttributeChangedCallback(meshNode.parent(0), transformAttributeChanged));
			idArray.append(MNodeMessage::addNodePreRemovalCallback(meshNode.object(), nodeRemoval));
		}
		else if (plugName.find("inMesh") != std::string::npos)
		{
			MPlugArray plugArray;
			if (p_Plug.connectedTo(plugArray, true, false))
				idArray.append(MNodeMessage::addAttributeChangedCallback(plugArray[0].node(), test));
		}
		else if (p_Plug.isArray() && p_Plug == meshNode.findPlug("pnts"))
		{
			meshVerteciesChanged(p_Plug); 
		}
		else if (plugName.find("instObjGroups") != std::string::npos)
		{
			meshy(p_Plug.node());
		}
	}
}

void textureChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &p_Plug2, void *p_ClientData)
{
		//MPlug plug = depNode.findPlug("color", true);
		MObject object;
		MPlugArray allConnections;
		plug.connectedTo(allConnections, false, true);

		for (unsigned int i = 0; i < allConnections.length(); i++)
		{
			object = allConnections[i].node();
			if (object.hasFn(MFn::kLambert))
				materialChange(object);
		}	
}

void materialChange(MObject &node)
{
	MFnDependencyNode depNode(node);
	MGlobal::displayInfo(depNode.name());
	MGlobal::displayInfo(node.apiTypeStr());
	float materialColor[3];
	float ambientColor[3];
	MString materialName;
	MString texturePath;
	MObject object;
	MPlugArray allConnections;

	if (node.hasFn(MFn::kLambert))
	{
		MFnLambertShader lambertShader(node);
		MColor color = lambertShader.color();
		materialColor[0] = color.r;
		materialColor[1] = color.g;
		materialColor[2] = color.b;

		color = lambertShader.ambientColor();
		color.get(ambientColor);

		MPlug plug = depNode.findPlug("color", true);
		plug.connectedTo(allConnections, true, false);

		for (unsigned int i = 0; i < allConnections.length(); i++)
		{
			object = allConnections[i].node();
			if (object.hasFn(MFn::kFileTexture))
			{
				MFnDependencyNode texture_node(object);
				MPlug filename = texture_node.findPlug("ftn");

				texturePath =  filename.asString().asChar();
				MGlobal::displayInfo(MString() + "TEXTURE PATH: " + texturePath);
				break;
			}
		}
		materialName = lambertShader.name();
	}

	if (node.hasFn(MFn::kPhong))
	{
		MGlobal::displayInfo(MString() + "BLINN!!!");
	}


	MaterialHeader header;
	header.materialNameLength = materialName.length() + 1;
	if (texturePath.length() == 0)
		header.texturePathLength = 0;
	else
		header.texturePathLength = texturePath.length() + 1;

	UINT64 offset = 0;
	char *&data = mem.getAllocatedMemory(sizeof(MessageType::mNewMaterial) + sizeof(MaterialHeader) + header.materialNameLength + header.texturePathLength + sizeof(float) * 3 + sizeof(float) * 3);
	MessageType type = MessageType::mNewMaterial;

	memcpy(data, &type, sizeof(MessageType::mNewMaterial));
	offset += sizeof(MessageType::mNewMaterial);

	memcpy(&data[offset], &header, sizeof(MaterialHeader));
	offset += sizeof(MaterialHeader);

	memcpy(&data[offset], materialName.asChar(), header.materialNameLength);
	offset += header.materialNameLength;

	memcpy(&data[offset], texturePath.asChar(), header.texturePathLength);
	offset += header.texturePathLength;

	memcpy(&data[offset], ambientColor, sizeof(float) * 3);
	offset += sizeof(float) * 3;

	memcpy(&data[offset], materialColor, sizeof(float)* 3);
	offset += sizeof(float)* 3;

	if (messageQueue.empty())
	{
		if (gShared.write(data, offset))
		{
			return;
		}
	}
	MessageQueueStruct queueData(data, offset);
	messageQueue.push_back(queueData);
}
void materialAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData)
{
	MGlobal::displayInfo(MString() + " MAT CHANGE!!!");

	materialChange(p_Plug.node());
}

void materialCreated(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData)
{
	MObject node = p_Plug.node();
	if (node.apiType() == MFn::kLambert)
	{
		MGlobal::displayInfo("materialCreated");
		MGlobal::displayInfo(node.apiTypeStr());
		MFnDependencyNode tmp(node);
		MGlobal::displayInfo(tmp.name());
		idArray.append(MNodeMessage::addAttributeChangedCallback(node, materialAttributeChanged));
		materialChange(node);
	}
}

void test(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData)
{
	std::string plugName(p_Plug.name().asChar());
	
	if (p_Msg & MNodeMessage::AttributeMessage::kAttributeSet && plugName.find("cacheInput") == std::string::npos)
	{
		MPlugArray plugArray;
		MFnDependencyNode thisNode(p_Plug.node());
		MGlobal::displayInfo("HEJ HALLÅ: " + p_Plug.name());
		MGlobal::displayInfo(p_Plug.node().apiTypeStr());
		if (thisNode.findPlug("output").connectedTo(plugArray, false, true))
		{
			MFnDependencyNode tmp(plugArray[0].node());
			MGlobal::displayInfo(tmp.name());
			meshCreated(plugArray[0].node());
		}
		else
		{
			return;
		}
		
		//meshCreated(*tmp);
	}
}

void meshCreated(MObject &node)
{
	MFnMesh meshNode(node);

	map<UINT, vector<UINT>> vertexToIndex;

	vector<VertexLayout> verteciesData;	
	vector<UINT>indexArray;

	MIntArray intdexOffsetVertexCount, vertecies, triangleList;
	MPointArray dummy;

	UINT vertexIndex;
	MVector normal;
	MPoint pos;
	float2 UV;
	VertexLayout thisVertex;
	for (MItMeshPolygon meshPolyIter(node); !meshPolyIter.isDone(); meshPolyIter.next())
	{
		vector<UINT> localVertexToGlobalIndex;
		meshPolyIter.getVertices(vertecies);

		meshPolyIter.getTriangles(dummy, triangleList);
		UINT indexOffset = verteciesData.size();

		//MItMeshPolygon polygon(node);
		//int dummyInt;

		//for (UINT i = 0; i < meshPolyIter.index(); i++)
		//{
		//	polygon.setIndex(i, dummyInt);
		//	polygon.getVertices(intdexOffsetVertexCount);
		//	indexOffset += intdexOffsetVertexCount.length();
		//}
		//
		for (UINT i = 0; i < vertecies.length(); i++)
		{
			vertexIndex = meshPolyIter.vertexIndex(i);
			pos = meshPolyIter.point(i);
			pos.get(thisVertex.pos);

			meshPolyIter.getNormal(i, normal);
			thisVertex.normal[0] = normal[0];
			thisVertex.normal[1] = normal[1];
			thisVertex.normal[2] = normal[2];

			meshPolyIter.getUV(i, UV);
			thisVertex.texcoord[0] = UV[0];
			thisVertex.texcoord[1] = UV[1];

			verteciesData.push_back(thisVertex);
			localVertexToGlobalIndex.push_back(vertexIndex);
		}
		for (UINT i = 0; i < triangleList.length(); i++)
		{
			UINT k = 0;
			while (localVertexToGlobalIndex[k] != triangleList[i])
				k++;
			indexArray.push_back(indexOffset + k);
		}
	}


	MeshHeader meshHeader;
	meshHeader.nameLength = meshNode.name().length() + 1;
	meshHeader.vertexCount = verteciesData.size();
	meshHeader.indexCount = indexArray.size();

	char*& data = mem.getAllocatedMemory(sizeof(MessageType::mNewMesh) + sizeof(MeshHeader)+meshNode.name().length() + 1 + sizeof(verteciesData[0]) * verteciesData.size() + sizeof(int)* meshHeader.indexCount);

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
	memcpy(&data[offset], indexArray.data(), sizeof(int)* indexArray.size());
	offset += sizeof(int)* indexArray.size();


	idArray.append(MNodeMessage::addAttributeChangedCallback(meshNode.parent(0), transformAttributeChanged));
	idArray.append(MNodeMessage::addNodePreRemovalCallback(node, nodeRemoval));

	if (messageQueue.size() == 0)
	{
		if (gShared.write(data, offset))
		{
			return;
		}
	}
	MessageQueueStruct queueData(data, offset);
	messageQueue.push_back(queueData);

	//vertexIndexMap[meshNode.fullPathName().asChar()] = vertexToIndex;

}

void meshVerteciesChanged(MPlug &plug)
{
	MFnMesh meshNode(plug.node());

	vector<VertexLayout> verteciesData;
	vector<UINT>indexArray;

	MIntArray intdexOffsetVertexCount, vertecies, triangleList;
	MPointArray dummy;

	UINT vertexIndex;
	MVector normal;
	MPoint pos;
	float2 UV;
	VertexLayout thisVertex;
	for (MItMeshPolygon meshPolyIter(plug.node()); !meshPolyIter.isDone(); meshPolyIter.next())
	{
		vector<UINT> localVertexToGlobalIndex;
		meshPolyIter.getVertices(vertecies);

		meshPolyIter.getTriangles(dummy, triangleList);
		UINT indexOffset = verteciesData.size();

		//MItMeshPolygon polygon(node);
		//int dummyInt;

		//for (UINT i = 0; i < meshPolyIter.index(); i++)
		//{
		//	polygon.setIndex(i, dummyInt);
		//	polygon.getVertices(intdexOffsetVertexCount);
		//	indexOffset += intdexOffsetVertexCount.length();
		//}
		//
		for (UINT i = 0; i < vertecies.length(); i++)
		{
			vertexIndex = meshPolyIter.vertexIndex(i);
			pos = meshPolyIter.point(i);
			pos.get(thisVertex.pos);

			meshPolyIter.getNormal(i, normal);
			thisVertex.normal[0] = normal[0];
			thisVertex.normal[1] = normal[1];
			thisVertex.normal[2] = normal[2];

			meshPolyIter.getUV(i, UV);
			thisVertex.texcoord[0] = UV[0];
			thisVertex.texcoord[1] = UV[1];

			verteciesData.push_back(thisVertex);
			localVertexToGlobalIndex.push_back(vertexIndex);
		}
		for (UINT i = 0; i < triangleList.length(); i++)
		{
			UINT k = 0;
			while (localVertexToGlobalIndex[k] != triangleList[i])
				k++;
			indexArray.push_back(indexOffset + k);
		}
	}

	VertexChangeHeader header;
	header.nameLength = meshNode.name().length() + 1;
	header.numVerteciesChanged = verteciesData.size();

	char*& data = mem.getAllocatedMemory(sizeof(MessageType::mVertexChange) + sizeof(VertexChangeHeader) + meshNode.name().length() + 1 + sizeof(verteciesData[0]) * verteciesData.size());

	MessageType type = MessageType::mVertexChange;

	UINT64 offset = 0;

	memcpy(&data[offset], &type, sizeof(MessageType::mVertexChange));
	offset += sizeof(MessageType::mVertexChange);
	// -- Copy mesh header;
	memcpy(&data[offset], &header, sizeof(VertexChangeHeader));
	offset += sizeof(VertexChangeHeader);
	// -- Copy name;
	memcpy(&data[offset], meshNode.name().asChar(), meshNode.name().length() + 1);
	offset += meshNode.name().length() + 1;
	// -- Copy vertecies;
	memcpy(&data[offset], verteciesData.data(), sizeof(verteciesData[0]) * verteciesData.size());
	offset += sizeof(verteciesData[0]) * verteciesData.size();
	// -- Copy indecies;
	//memcpy(&data[offset], indexArray.data(), sizeof(int)* indexArray.size());
	//offset += sizeof(int)* indexArray.size();

	if (messageQueue.size() == 0)
	{
		if (gShared.write(data, offset))
		{
			return;
		}
	}
	MessageQueueStruct queueData(data, offset);
	messageQueue.push_back(queueData);
}

void cameraChanged(const MString &str, MObject &node, void *clientData)
{
	MFnDagNode thisNode(node);
	MString name = thisNode.name();
	MessageType type = MessageType::mCameraChanged;

	NameHeader header;
	header.nameLength = name.length() + 1;


	char*& data = mem.getAllocatedMemory(sizeof(MessageType::mCameraChanged) + sizeof(NameHeader)+thisNode.name().length() + 1);
	UINT64 offset = 0;
	memcpy(&data[offset], &type, sizeof(MessageType::mCameraChanged));
	offset += sizeof(MessageType::mCameraChanged);

	memcpy(&data[offset], &header, sizeof(NameHeader));
	offset += sizeof(NameHeader);
	// -- Copy name;
	memcpy(&data[offset], name.asChar(), name.length() + 1);
	offset += name.length() + 1;

	if (messageQueue.size() == 0)
	{
		if (gShared.write(data, offset))
		{
			return;
		}
	}
	MessageQueueStruct queueData(data, offset);
	messageQueue.push_back(queueData);
}

void cameraAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData)
{
	MFnCamera camera(p_Plug.node());
	if (p_Msg & MNodeMessage::kAttributeSet)
	{
		cameraCreated(p_Plug.node());
	}
}

void cameraCreated(MObject &node)
{
	MFnCamera camera(node);
	NodeRemovedHeader camHeader;
	camHeader.nameLength = camera.name().length() + 1;

	//bool isOrtho;
	//isOrtho = camera.isOrtho();
	char *&data = mem.getAllocatedMemory(sizeof(MessageType::mCamera) + sizeof(NodeRemovedHeader) + camera.name().length() + (sizeof(float)* 4 * 4)/* + sizeof(isOrtho) + sizeof(float) * 4*/);

	MFloatMatrix projectionMatrix = camera.projectionMatrix();
	float camMatrix[4][4];
	projectionMatrix.get(camMatrix);

	/*float nearPlane = camera.nearClippingPlane();
	float farPlane = camera.farClippingPlane();
	float aspectRatio = camera.aspectRatio();
	float fov = camera.horizontalFieldOfView();*/

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

	/*memcpy(&data[offset], &isOrtho, sizeof(isOrtho));
	offset += sizeof(isOrtho);

	memcpy(&data[offset], &nearPlane, sizeof(float));
	offset += sizeof(float);

	memcpy(&data[offset], &farPlane, sizeof(float));
	offset += sizeof(float);

	memcpy(&data[offset], &aspectRatio, sizeof(float));
	offset += sizeof(float);

	memcpy(&data[offset], &fov, sizeof(float));
	offset += sizeof(float);*/

	idArray.append(MNodeMessage::addAttributeChangedCallback(camera.parent(0), transformAttributeChanged));
	idArray.append(MNodeMessage::addNodePreRemovalCallback(node, nodeRemoval));

	if (messageQueue.size() == 0)
	{
		if (gShared.write(data, offset))
		{
			return;
		}
	}
	MessageQueueStruct queueData(data, offset);
	messageQueue.push_back(queueData);


}

void lightAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData)
{
	MFnPointLight pointLight(p_Plug);
	if (p_Msg & MNodeMessage::kAttributeSet)
		lightChanged(p_Plug.node());

}

void lightChanged(MObject &node)
{
	MFnPointLight pointLight(node);
	MColor color;
	color = pointLight.color();
	float rbgValues[3];
	rbgValues[0] = color.r;
	rbgValues[1] = color.g;
	rbgValues[2] = color.b;

	float lightRange;
	lightRange = pointLight.centerOfIllumination();
	//pointLight.findPlug("lightRadius").getValue(lightRange);

	MessageType type = MessageType::mLight;

	char *&data = mem.getAllocatedMemory(sizeof(MessageType::mLight) + sizeof(float) * 4);

	UINT64 offset = 0;

	memcpy(data, &type, sizeof(MessageType::mLight));
	offset += sizeof(MessageType::mLight);

	memcpy(&data[offset], rbgValues, sizeof(float) * 3);
	offset += sizeof(float) * 3;

	memcpy(&data[offset], &lightRange, sizeof(float));
	offset += sizeof(float);

	idArray.append(MNodeMessage::addAttributeChangedCallback(pointLight.parent(0), transformAttributeChanged));
	idArray.append(MNodeMessage::addNodePreRemovalCallback(node, nodeRemoval));

	if (messageQueue.size() == 0)
	{
		if (gShared.write(data, offset))
		{
			return;
		}
	}
	MessageQueueStruct queueData(data, offset);
	messageQueue.push_back(queueData);
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

	if (node.hasFn(MFn::kPointLight))
	{
		idArray.append(MNodeMessage::addAttributeChangedCallback(node, lightAttributeChanged, clientData));
	}

	if (node.hasFn(MFn::kLambert))
	{
		idArray.append(MNodeMessage::addAttributeChangedCallback(node, materialCreated));
	}

	if (node.hasFn(MFn::kFileTexture))
	{
		MGlobal::displayInfo("FileTextureAdded");
		idArray.append(MNodeMessage::addAttributeChangedCallback(node, textureChanged));
	}
}
void nodeRemoval(MObject &node, void *clientData)
{
	if (node.hasFn(MFn::kMesh) || node.hasFn(MFn::kCamera))
	{
		MFnDependencyNode tmp(node);

		NodeRemovedHeader header;
	
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
		{
			if (gShared.write(data, offset))
			{
				return;
			}
		}
		MessageQueueStruct queueData(data, offset);
		messageQueue.push_back(queueData);
	}
}

void timer(float elapsedTime, float lastTime, void *clientData)
{
	if (messageQueue.size() > 0)
		while (gShared.write(messageQueue.front().data, messageQueue.front().size))
			messageQueue.pop_front();
}