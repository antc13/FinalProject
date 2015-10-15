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

map<string, map<UINT, vector<UINT>>> vertexIndexMap;

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
		else if (p_Plug.isArray() && p_Plug == MFnMesh(p_Plug.node()).findPlug("pnts") && p_Msg & MNodeMessage::AttributeMessage::kAttributeSet)
		{
			meshVerteciesChanged(p_Plug);
		}
	}
}

void meshCreated(MObject node)
{
	MFnMesh meshNode(node);
	map<UINT, vector<UINT>> vertexToIndex;
//
//	//Variabler med VextrePos Data
//	MFloatPointArray vertexPos;
//	meshNode.getPoints(vertexPos, MSpace::kObject);
//
//	//Variabler med Normal Data
//	//vector<float*> normals;
//
//	//Variabler med UV data
//	MFloatArray u_texture;
//	MFloatArray v_texture;
//	meshNode.getUVs(u_texture, v_texture);
//	vector<int>uvIDs;
//
//	//Variabler med Index Data
//	MIntArray triPerPolygonCount;
//	MIntArray triVertecies;
//	int* triVerteciesAsInt= nullptr;
//	meshNode.getTriangles(triPerPolygonCount, triVertecies);
//	triVerteciesAsInt = new int[triVertecies.length()];
//	triVertecies.get(triVerteciesAsInt);
//	vector<UINT>indexArray;
//
//	for (UINT i = 0; i < triPerPolygonCount.length(); i++)
//	{
//		for (UINT k = 0; k < 3; k++)
//		{
//			int tmp;
//			meshNode.getPolygonUVid(i, k, tmp);
//			uvIDs.push_back(tmp);
//		}
//	}
//	//int* triVerticesArray;
//	//triVerticesArray = new int[triVertices.length()];
//	//triVertices.get(triVerticesArray);
//
//	//------------ TEST ------------ TEST ------------ TEST ------------ TEST ------------ TEST ------------ TEST ------------ TEST ------------ TEST 
//
//	//UINT normalOffset = 0;
//	//MVector tmpNormalVector;
//	//double tmpNormalDouble[3];
//
//	//for (UINT i = 0; i < meshNode.numPolygons(); i++)
//	//{
//	//	MFloatVectorArray normalsTest;
//	//	for (UINT k = 0; k < triPerPolygonCount[i]; k++)
//	//	{
//	//		for (UINT L = 0; L < 3; L++)
//	//		{
//	//			float* tmpNormalFloat = new float[3];
//	//			meshNode.getFaceVertexNormal(i, triVertecies[normalOffset], tmpNormalVector);
//	//			tmpNormalVector.get(tmpNormalDouble);
//	//			tmpNormalFloat[0] = tmpNormalDouble[0];
//	//			tmpNormalFloat[1] = tmpNormalDouble[1];
//	//			tmpNormalFloat[2] = tmpNormalDouble[2];
//	//			normals.push_back(tmpNormalFloat);
//	//			//MGlobal::displayInfo(MString() + tmpNormalFloat[0] + " " + tmpNormalFloat[1] + " " + tmpNormalFloat[2]);
//	//			normalOffset++;
//	//		}
//	//	}
//	//}
//
//	//-------- END TEST -------- END TEST -------- END TEST -------- END TEST -------- END TEST -------- END TEST -------- END TEST -------- END TEST
//
//	vector<VertexLayout> verteciesData;
//
//	VertexLayout thisVertex;
//
//	MFloatVectorArray normals;
//	meshNode.getNormals(normals);
//	MVector Normal;
//	float normalF[3];
//	MItMeshVertex vertexIter(meshNode.object());
//	
//	for (UINT i = 0; i < triVertecies.length(); i++)
//	{
//		normals[triVertecies[i]].get(thisVertex.normal);
//		vertexPos[triVertecies[i]].get(thisVertex.pos);
//		//thisVertex.normal[0] = normals.at(i)[0];
//		//thisVertex.normal[1] = normals.at(i)[1];
//		//thisVertex.normal[2] = normals.at(i)[2];
//		//MGlobal::displayInfo(MString() + thisVertex.pos[0] + " " + thisVertex.pos[1] + " " + thisVertex.pos[2] + " " + thisVertex.normal[0] + " " + thisVertex.normal[1] + " " + thisVertex.normal[2]);
//		bool newVertex = true;
//		vector<VertexLayout>::iterator it;
//		UINT index = 0;
//		for (it = verteciesData.begin(); it < verteciesData.end(); it++)
//		{
//
//			if (it->pos[0] == thisVertex.pos[0] && it->pos[1] == thisVertex.pos[1] && it->pos[2] == thisVertex.pos[2] &&
//				it->normal[0] == thisVertex.normal[0] && it->normal[1] == thisVertex.normal[1] && it->normal[2] == thisVertex.normal[2])
//			{
//				if (!(find(vertexToIndex[triVertecies[i]].begin(), vertexToIndex[triVertecies[i]].end(), index) != vertexToIndex[triVertecies[i]].end()))
//					vertexToIndex[triVertecies[i]].push_back(index);
//				indexArray.push_back(index);
//				newVertex = false;
//				break;
//			}
//			index++;
//		}
//		if (newVertex)
//		{
//			if (!(find(vertexToIndex[triVertecies[i]].begin(), vertexToIndex[triVertecies[i]].end(), index) != vertexToIndex[triVertecies[i]].end()))
//				vertexToIndex[triVertecies[i]].push_back(index);
//			//MGlobal::displayInfo(MString() + thisVertex.pos[0] + " " + thisVertex.pos[1] + " " + thisVertex.pos[2] + " " + thisVertex.normal[0] + " " + thisVertex.normal[1] + " " + thisVertex.normal[2]);
//			verteciesData.push_back(thisVertex);
//			indexArray.push_back(index);
//		}
//	}
//	vector<float*>::iterator it;
////	for (it = normals.begin(); it < normals.end(); it++)
////	{
////		delete[] (*it);
////	}
	vector<VertexLayout> verteciesData;	
	vector<UINT>indexArray;

	MIntArray TrianglePerPolygon, TriangleVertexList;
	meshNode.getTriangles(TrianglePerPolygon, TriangleVertexList);

	UINT TriangleVertexListOffset = 0;
	for (int polygon = 0; polygon < meshNode.numPolygons(); polygon++)
	{
		VertexLayout thisVertex;
		MPoint thisPosition;
		MVector thisNormal;
		double thisNormalAsdouble[3];
		for (int triangle = 0; triangle < TrianglePerPolygon[polygon]; triangle++)
		{
			for (int vertex = 0; vertex < 3; vertex++)
			{
				meshNode.getPoint(TriangleVertexList[TriangleVertexListOffset], thisPosition);
				meshNode.getFaceVertexNormal(polygon, TriangleVertexList[TriangleVertexListOffset], thisNormal);
				thisPosition.get(thisVertex.pos);
				thisNormal.get(thisNormalAsdouble);

				thisVertex.normal[0] = thisNormalAsdouble[0];
				thisVertex.normal[1] = thisNormalAsdouble[1];
				thisVertex.normal[2] = thisNormalAsdouble[2];

				bool newVertex = true;
				UINT index = 0;
				vector<VertexLayout>::iterator it;
				for (it = verteciesData.begin(); it < verteciesData.end(); it++)
				{

					if (it->pos[0] == thisVertex.pos[0] && it->pos[1] == thisVertex.pos[1] && it->pos[2] == thisVertex.pos[2] &&
						it->normal[0] == thisVertex.normal[0] && it->normal[1] == thisVertex.normal[1] && it->normal[2] == thisVertex.normal[2])
					{
						if (!(find(vertexToIndex[TriangleVertexList[TriangleVertexListOffset]].begin(), vertexToIndex[TriangleVertexList[TriangleVertexListOffset]].end(), index) != vertexToIndex[TriangleVertexList[TriangleVertexListOffset]].end()))
							vertexToIndex[TriangleVertexList[TriangleVertexListOffset]].push_back(index);
						indexArray.push_back(index);
						newVertex = false;
						break;
					}
					index++;
				}
				if (newVertex)
				{
					if (!(find(vertexToIndex[TriangleVertexList[TriangleVertexListOffset]].begin(), vertexToIndex[TriangleVertexList[TriangleVertexListOffset]].end(), index) != vertexToIndex[TriangleVertexList[TriangleVertexListOffset]].end()))
						vertexToIndex[TriangleVertexList[TriangleVertexListOffset]].push_back(index);
					//MGlobal::displayInfo(MString() + thisVertex.pos[0] + " " + thisVertex.pos[1] + " " + thisVertex.pos[2] + " " + thisVertex.normal[0] + " " + thisVertex.normal[1] + " " + thisVertex.normal[2]);
					verteciesData.push_back(thisVertex);
					indexArray.push_back(index);
				}

				TriangleVertexListOffset++;
			}
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

	if (messageQueue.size() == 0)
		gShared.write(data, offset);
	else
	{
		MessageQueueStruct queueData(data, offset);
		messageQueue.push_back(queueData);
	}

	idArray.append(MNodeMessage::addAttributeChangedCallback(meshNode.parent(0), transformAttributeChanged));
	idArray.append(MNodeMessage::addNodePreRemovalCallback(node, nodeRemoval));
	vertexIndexMap[meshNode.fullPathName().asChar()] = vertexToIndex;
}

void meshVerteciesChanged(MPlug plug)
{
	//int index = plug.logicalIndex();
	////MGlobal::displayInfo(plug.name());
	////MGlobal::displayInfo(plug.node().apiTypeStr());
	MFnMesh meshNode(plug.node());

	//MFloatPointArray vertexPos;
	//meshNode.getPoints(vertexPos, MSpace::kObject);

	//VertexLayout thisVertex;
	//vertexPos[index].get(thisVertex.pos);
	//vector<UINT> thisVertexIndecies = vertexIndexMap[meshNode.fullPathName().asChar()][index];
	//MFloatVectorArray normals;
	//meshNode.getNormals(normals);
	//MVector Normal;
	//float normalF[3];
	//vector<VertexLayout> verteciesData;
	//MItMeshVertex vertexIter(meshNode.object());
	//for (UINT i = 0; i < thisVertexIndecies.size(); i++)
	//{
	//	normals[i].get(thisVertex.normal);

	//	verteciesData.push_back(thisVertex);
	//}


	vector<VertexLayout> verteciesData;
	vector<UINT>indexArray;

	MIntArray TrianglePerPolygon, TriangleVertexList;
	meshNode.getTriangles(TrianglePerPolygon, TriangleVertexList);

	UINT TriangleVertexListOffset = 0;
	for (int polygon = 0; polygon < meshNode.numPolygons(); polygon++)
	{
		VertexLayout thisVertex;
		MPoint thisPosition;
		MVector thisNormal;
		double thisNormalAsdouble[3];
		for (int triangle = 0; triangle < TrianglePerPolygon[polygon]; triangle++)
		{
			for (int vertex = 0; vertex < 3; vertex++)
			{
				meshNode.getPoint(TriangleVertexList[TriangleVertexListOffset], thisPosition);
				meshNode.getFaceVertexNormal(polygon, TriangleVertexList[TriangleVertexListOffset], thisNormal);
				thisPosition.get(thisVertex.pos);
				thisNormal.get(thisNormalAsdouble);

				thisVertex.normal[0] = thisNormalAsdouble[0];
				thisVertex.normal[1] = thisNormalAsdouble[1];
				thisVertex.normal[2] = thisNormalAsdouble[2];

				bool newVertex = true;
				vector<VertexLayout>::iterator it;
				for (it = verteciesData.begin(); it < verteciesData.end(); it++)
				{

					if (it->pos[0] == thisVertex.pos[0] && it->pos[1] == thisVertex.pos[1] && it->pos[2] == thisVertex.pos[2] &&
						it->normal[0] == thisVertex.normal[0] && it->normal[1] == thisVertex.normal[1] && it->normal[2] == thisVertex.normal[2])
					{
						newVertex = false;
						break;
					}
				}
				if (newVertex)
					verteciesData.push_back(thisVertex);
				TriangleVertexListOffset++;
			}
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
	//memcpy(&data[offset], thisVertexIndecies.data(), sizeof(int)* thisVertexIndecies.size());
	//offset += sizeof(int)* thisVertexIndecies.size();

	if (messageQueue.size() == 0)
		gShared.write(data, offset);
	else
	{
		MessageQueueStruct queueData(data, offset);
		messageQueue.push_back(queueData);
	}

	//MFnMesh meshNode(node);
	//MPointArray verticesPos;
	//meshNode.getPoints(verticesPos, MSpace::kObject);



	////vector<int> facesModified;

	////MStringArray commandResult;

	//////------- This makes so that whatever the user selects we also update the surrounding vertecies
	//////------- Get whatever is selected as vertecies if posible, if no component is selected, it will fail
	////if (MGlobal::executePythonCommand("maya.cmds.polyListComponentConversion(tv=True)", commandResult))
	////{
	////	//------- Select All Vertecies
	////	//MGlobal::executePythonCommand("maya.cmds.select('" + commandResult[0] + "' , r = True)");
	////	for (UINT i = ; i < commandResult.length(); i++)
	////	{
	////		MGlobal::executePythonCommand("maya.cmds.select('" + commandResult[i] + "', add=True )");
	////	}
	////	//------- Get the faces that shares atleas on point of the selected vertecies
	////	MGlobal::executePythonCommand("maya.cmds.polyListComponentConversion(tf=True)", commandResult);

	////	//------- Get the selected faces from commandResult
	////	for (UINT i = 0; i < commandResult.length(); i++)
	////	{
	////		MGlobal::displayInfo(MString() + commandResult[i]);
	////		const char* thisString = commandResult[i].asChar();
	////		UINT thisStringLength = commandResult[i].length() - 1;
	////		UINT stringOffset = 0;
	////		int lastFace = 0;
	////		int firstFace = -1;
	////		if (thisString[thisStringLength] == ']')
	////		{
	////			stringOffset += 1;
	////			vector<char>tmpLastFace;
	////			tmpLastFace.push_back('\0');
	////			while (thisString[thisStringLength - stringOffset] == '0' || thisString[thisStringLength - stringOffset] == '1' ||
	////				thisString[thisStringLength - stringOffset] == '2' || thisString[thisStringLength - stringOffset] == '3' ||
	////				thisString[thisStringLength - stringOffset] == '4' || thisString[thisStringLength - stringOffset] == '5' ||
	////				thisString[thisStringLength - stringOffset] == '6' || thisString[thisStringLength - stringOffset] == '7' ||
	////				thisString[thisStringLength - stringOffset] == '8' || thisString[thisStringLength - stringOffset] == '9')
	////			{
	////				tmpLastFace.push_back(thisString[thisStringLength - stringOffset]);
	////				stringOffset += 1;
	////			}
	////			stringOffset += 1;
	////			char* tmp = new char[tmpLastFace.size()];
	////			vector<char>::reverse_iterator it;
	////			UINT z = 0;
	////			for (it = tmpLastFace.rbegin(); it != tmpLastFace.rend(); it++)
	////			{
	////				tmp[z] = *it;
	////				z++;
	////			}
	////			MString tmpString(tmp);
	////			if (tmpString.length() != 0)
	////				lastFace = tmpString.asInt();
	////			delete[] tmp;
	////			tmpString.clear();

	////			tmpLastFace.clear();
	////			tmpLastFace.push_back('\0');
	////			while (thisString[thisStringLength - stringOffset] == '0' || thisString[thisStringLength - stringOffset] == '1' ||
	////				thisString[thisStringLength - stringOffset] == '2' || thisString[thisStringLength - stringOffset] == '3' ||
	////				thisString[thisStringLength - stringOffset] == '4' || thisString[thisStringLength - stringOffset] == '5' ||
	////				thisString[thisStringLength - stringOffset] == '6' || thisString[thisStringLength - stringOffset] == '7' ||
	////				thisString[thisStringLength - stringOffset] == '8' || thisString[thisStringLength - stringOffset] == '9')
	////			{
	////				tmpLastFace.push_back(thisString[thisStringLength - stringOffset]);
	////				stringOffset += 1;
	////			}

	////			tmp = new char[tmpLastFace.size()];
	////			z = 0;
	////			for (it = tmpLastFace.rbegin(); it != tmpLastFace.rend(); it++)
	////			{
	////				tmp[z] = *it;
	////				z++;
	////			}
	////			tmpString.set(tmp);
	////			if (tmpString.length() != 0)
	////				firstFace = tmpString.asInt();
	////			delete[] tmp;

	////			if (firstFace == -1)
	////				facesModified.push_back(lastFace);
	////			else
	////			{
	////				for (UINT k = firstFace; k <= lastFace; k++)
	////					facesModified.push_back(k);
	////			}
	////		}
	////	}
	//	vector<int>FaceIDsToUpdate;

	////	vector<int>::iterator it;
	////	MIntArray VerteciesToUpdate;
	////	UINT z = 0;
	////	for (it = facesModified.begin(); it != facesModified.end(); it++)
	////	{
	////		meshNode.getPolygonVertices(*it, VerteciesToUpdate);
	////		for (unsigned int i = 0; i < VerteciesToUpdate.length(); i++)
	////		{
	////			if (!(find(FaceIDsToUpdate.begin(), FaceIDsToUpdate.end(), VerteciesToUpdate[i]) != FaceIDsToUpdate.end()))
	////			{
	////				FaceIDsToUpdate.push_back(VerteciesToUpdate[i]);
	////				//for(UINT W = 0; W < vertexIndexMap[meshNode.fullPathName().asChar()].at(VerteciesToUpdate[i]).size(); W++)
	////				//	FaceIDsToUpdate.push_back(vertexIndexMap[meshNode.fullPathName().asChar()].at(VerteciesToUpdate[i])[W]);
	////			}
	////		}
	////	}

	//	//Iterate tought selected
	//	MRichSelection activeList;
	//	MGlobal::getRichSelection(activeList, true);
	//	MSelectionList list1;
	//	MSelectionList list2;
	//	activeList.getSelection(list1);
	//	activeList.getSymmetry(list2);
	//	list1.merge(list2, MSelectionList::kMergeNormal);
	//	MItSelectionList iter(list1);
	//	for (; !iter.isDone(); iter.next())
	//	{
	//		MDagPath item;
	//		MObject components;
	//		iter.getDagPath(item, components);

	//		//if currentNode in iter == this node then do stuff
	//		if (item.node() == node)
	//		{
	//			if (components.apiType() == MFn::kMeshVertComponent)
	//			{
	//				INT dummy;
	//				MItMeshVertex vertexIter(item, components);
	//				for (; !vertexIter.isDone(); vertexIter.next())
	//				{
	//					MIntArray connectedFaces;
	//					vertexIter.getConnectedFaces(connectedFaces);
	//					for (UINT f = 0; f < connectedFaces.length(); f++)
	//					{
	//						if (!(find(FaceIDsToUpdate.begin(), FaceIDsToUpdate.end(), connectedFaces[f]) != FaceIDsToUpdate.end()))
	//						{
	//							MGlobal::displayInfo(MString() + connectedFaces[f]);
	//							FaceIDsToUpdate.push_back(connectedFaces[f]);
	//						}
	//					}
	//				}
	//			}
	//			else if (components.apiType() == MFn::kMeshEdgeComponent)
	//			{
	//				INT dummy;
	//				MItMeshEdge edgeIter(item, components);
	//				for (; !edgeIter.isDone(); edgeIter.next())
	//				{
	//					MIntArray connectedEdges;
	//					edgeIter.getConnectedEdges(connectedEdges);
	//					MItMeshEdge secondEdgeIter(item, components);
	//					for (UINT e = 0; e < connectedEdges.length(); e++)
	//					{	
	//						secondEdgeIter.setIndex(connectedEdges[e], dummy);
	//						MIntArray connectedFaces;
	//						secondEdgeIter.getConnectedFaces(connectedFaces);
	//						MItMeshPolygon faceIter(item);
	//						for (UINT f = 0; f < connectedFaces.length(); f++)
	//						{
	//							if (!(find(FaceIDsToUpdate.begin(), FaceIDsToUpdate.end(), connectedFaces[f]) != FaceIDsToUpdate.end()))
	//								{
	//									MGlobal::displayInfo(MString() + connectedFaces[f]);
	//									FaceIDsToUpdate.push_back(connectedFaces[f]);
	//								}
	//						}
	//					}
	//				}
	//				
	//			}
	//			else if (components.apiType() == MFn::kMeshPolygonComponent)
	//			{
	//				INT dummy;
	//				vector<int> alreadyCheackedFaces;
	//				MItMeshPolygon polyIter(item, components);
	//				for (; !polyIter.isDone(); polyIter.next())
	//				{
	//					MIntArray connectedEdges;
	//					polyIter.getConnectedEdges(connectedEdges);
	//					MItMeshEdge secondEdgeIter(item, components);
	//					for (UINT e = 0; e < connectedEdges.length(); e++)
	//					{
	//						secondEdgeIter.setIndex(connectedEdges[e], dummy);
	//						MIntArray connectedFaces;
	//						secondEdgeIter.getConnectedFaces(connectedFaces);
	//						MItMeshPolygon faceIter(item);
	//						for (UINT f = 0; f < connectedFaces.length(); f++)
	//						{
	//							if (!(find(FaceIDsToUpdate.begin(), FaceIDsToUpdate.end(), connectedFaces[f]) != FaceIDsToUpdate.end()))
	//							{
	//								MGlobal::displayInfo(MString() + connectedFaces[f]);
	//								FaceIDsToUpdate.push_back(connectedFaces[f]);
	//							}
	//						}
	//					}
	//				}
	//			}

	//			//----- Data

	//			MFloatPointArray vertexPos;
	//			meshNode.getPoints(vertexPos, MSpace::kObject);
	//			vector<VertexLayout>verteciesData;
	//			vector<int>indeciesUpdated;
	//			VertexLayout thisVertex;

	//			MVector tmpNormalVector;
	//			double tmpNormalDouble[3];

	//			MFloatVectorArray normalsTest;
	//			for (UINT faceID = 0; faceID < FaceIDsToUpdate.size(); faceID++)
	//			{
	//				MGlobal::displayInfo("writhing face to vectors");
	//				MIntArray vertexIDs;
	//				meshNode.getPolygonVertices(faceID, vertexIDs);
	//				for (UINT i = 0; i < vertexIDs.length(); i++)
	//				{
	//					MGlobal::displayInfo(MString() + "vertexIDs: " + vertexIDs[i]);
	//					vector<UINT> thisVertexIndecies = vertexIndexMap[meshNode.fullPathName().asChar()].at(vertexIDs[i]);
	//					MGlobal::displayInfo(MString() + "thisVertexIndecies.size(): " + thisVertexIndecies.size());
	//					for (UINT vertexToIndexNumber = 0; vertexToIndexNumber < thisVertexIndecies.size(); vertexToIndexNumber++)
	//					{		
	//						MGlobal::displayInfo(MString() + " thisVertexIndecies[vertexToIndexNumber]: " + thisVertexIndecies[vertexToIndexNumber]);
	//						meshNode.getFaceVertexNormal(faceID, thisVertexIndecies[vertexToIndexNumber], tmpNormalVector);
	//						MGlobal::displayInfo("TEST CHECK");
	//						vertexPos[vertexIDs[i]].get(thisVertex.pos);
	//						tmpNormalVector.get(tmpNormalDouble);
	//						thisVertex.normal[0] = tmpNormalDouble[0];
	//						thisVertex.normal[1] = tmpNormalDouble[1];
	//						thisVertex.normal[2] = tmpNormalDouble[2];
	//						verteciesData.push_back(thisVertex);
	//						indeciesUpdated.push_back(thisVertexIndecies[vertexToIndexNumber]);
	//					}
	//					//MGlobal::displayInfo(MString() + tmpNormalFloat[0] + " " + tmpNormalFloat[1] + " " + tmpNormalFloat[2]);
	//				}
	//			}


	//			////------ Send the Vertecies Changed ---------
	//			VertexChangeHeader header;
	//			header.nameLength = meshNode.name().length() + 1;
	//			header.numVerteciesChanged = indeciesUpdated.size();

	//			char* data = mem.getAllocatedMemory(sizeof(MessageType::mVertexChange) + sizeof(VertexChangeHeader) + meshNode.name().length() + 1 + indeciesUpdated.size() * sizeof(indeciesUpdated[0]) + verteciesData.size() * sizeof(verteciesData[0]));

	//			UINT64 offset = 0;
	//			MessageType type = MessageType::mVertexChange;

	//			memcpy(&data[offset], &type, sizeof(MessageType::mVertexChange));
	//			offset += sizeof(MessageType::mVertexChange);

	//			memcpy(&data[offset], &header, sizeof(VertexChangeHeader));
	//			offset += sizeof(VertexChangeHeader);

	//			memcpy(&data[offset], meshNode.name().asChar(), meshNode.name().length() + 1);
	//			offset += meshNode.name().length() + 1;

	//			memcpy(&data[offset], verteciesData.data(), verteciesData.size() * sizeof(verteciesData[0]));
	//			offset += verteciesData.size() * sizeof(verteciesData[0]);

	//			memcpy(&data[offset], indeciesUpdated.data(), indeciesUpdated.size() * sizeof(indeciesUpdated[0]));
	//			offset += indeciesUpdated.size() * sizeof(indeciesUpdated[0]);

	//			if (messageQueue.size() == 0)
	//				gShared.write(data, offset);
	//			else
	//			{
	//				MessageQueueStruct queueData(data, offset);
	//				messageQueue.push_back(queueData);
	//			}

	//			break;
	//		}
	//	}
	}
//}


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
	char *&data = mem.getAllocatedMemory(sizeof(MessageType::mCamera) + sizeof(float)* 4 * 4);

	MFloatMatrix projectionMatrix = camera.projectionMatrix();
	float camMatrix[4][4];
	projectionMatrix.get(camMatrix);

	MessageType type = MessageType::mCamera;
	memcpy(data, &type, sizeof(MessageType::mCamera));

	for (unsigned int i = 0; i < 4; i++)
		memcpy(&data[sizeof(MessageType::mCamera) + sizeof(float)* 4 * i], camMatrix[i], sizeof(float)* 4);

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
		nodeCallbackRemove(node);

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
		nodeCallbackRemove(node);
	}
}

void nodeCallbackRemove(MObject &node)
{
	MCallbackIdArray ids;
	MMessage::nodeCallbacks(node, ids);
	MMessage::removeCallbacks(ids);
}

void timer(float elapsedTime, float lastTime, void *clientData)
{
	if (messageQueue.size() > 0)
		while (gShared.write(messageQueue.front().data, messageQueue.front().size))
			messageQueue.pop_front();
}

void cameraChanged(const MString &str, MObject &node, void *clientData)
{
	MGlobal::displayInfo("HEJ");
}