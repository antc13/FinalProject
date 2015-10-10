#pragma once
#include "maya_includes.h"

void meshAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData);
void meshCreated(MFnMesh meshNode);
void nodeCreated(MObject &node, void *clientData);
void cameraAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData);


