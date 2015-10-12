#pragma once
#include "maya_includes.h"

void transformAttributeChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData);

void meshAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData);
void meshCreated(MObject node);
void meshVertecChanged(MObject meshNode);

void cameraCreated(MObject node);
void cameraAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData);

void nodeCreated(MObject &node, void *clientData);
void nodeRemoval(MObject &node, void *clientData);