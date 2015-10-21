// UD1414_Plugin.cpp : Defines the exported functions for the DLL application.

#include "maya_includes.h"
#include <maya/MFnPlugin.h>
#include <iostream>
#include "sharedMemory.h"
#include "callbackFunctions.h"

using namespace std;

SharedMemory gShared;
MCallbackIdArray idArray;

// called when the plugin is loaded
EXPORT MStatus initializePlugin(MObject obj)
{
	// most functions will use this variable to indicate for errors
	MStatus res = MS::kSuccess;

	MFnPlugin myPlugin(obj, "Maya plugin", "1.0", "Any", &res);
	if (MFAIL(res))  {
		CHECK_MSTATUS(res);
	}
	gShared.initialize(200 * 1024 * 1024, (LPCWSTR)"MayaToGameEngine", true);
	
	MDagPath path;

	MItDependencyNodes it(MFn::kLight);
	for (; !it.isDone(); it.next())
	{
		MObject node = it.thisNode();
		nodeCreated(node, nullptr);
		lightChanged(node);
		MFnDagNode thisNode(node);
		transformCreate(thisNode.parent(0));
	}

	it.reset(MFn::kLambert);
	for (; !it.isDone(); it.next())
	{
		MObject node = it.thisNode();
		nodeCreated(node, nullptr);
		materialCreated(node);
	}

	it.reset(MFn::kMesh);
	for (; !it.isDone(); it.next())
	{
		MObject node = it.thisNode();
		nodeCreated(node, nullptr);
		meshCreated(node);
		MFnDagNode thisNode(node);
		transformCreate(thisNode.parent(0));
		meshy(node);
	}

	it.reset(MFn::kCamera);
	for (; !it.isDone(); it.next())
	{
		MObject node = it.thisNode();
		nodeCreated(node, nullptr);
		cameraCreated(node);
		MFnDagNode thisNode(node);
		transformCreate(thisNode.parent(0));
	}

	M3dView view;
	M3dView::get3dView(0, view);
	MDagPath currentCamera;
	view.getCamera(currentCamera);
	MFnDagNode thisNode(currentCamera);
	cameraCreated(thisNode.object());
	transformCreate(thisNode.parent(0));

	idArray.append(MDGMessage::addNodeAddedCallback(nodeCreated));
	idArray.append(MUiMessage::addCameraChangedCallback("modelPanel4", cameraChanged));
	idArray.append(MTimerMessage::addTimerCallback(0.003, timer));
	idArray.append(MDGMessage::addNodeAddedCallback(materialCreated));

	MGlobal::displayInfo("Maya plugin loaded!");
	// if res == kSuccess then the plugin has been loaded,
	// otherwise is has not.
	return res;
}

EXPORT MStatus uninitializePlugin(MObject obj)
{
	// simply initialize the Function set with the MObject that represents
	// our plugin
	MFnPlugin plugin(obj);
	MMessage::removeCallbacks(idArray);
	// if any resources have been allocated, release and free here before
	// returning...
	MGlobal::displayInfo("Maya plugin unloaded!");

	return MS::kSuccess;
}
