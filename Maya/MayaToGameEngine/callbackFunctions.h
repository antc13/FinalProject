#pragma once
#include "maya_includes.h"

void transformAttributeChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData);
void transformCreate(MObject &node);

void meshAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData);
void meshCreated(MObject &node);
void meshVerteciesChanged(MPlug &plug);
void meshTopologyChange(MObject &node, void *clientData);

void cameraCreated(MObject &node);
void cameraAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData);
void cameraChanged(const MString &str, MObject &node, void *clientData);

void nodeCreated(MObject &node, void *clientData);
void nodeRemoval(MObject &node, void *clientData);
void nodeCallbackRemove(MObject &node);

void timer(float elapsedTime, float lastTime, void *clientData);

void lightChanged(MObject &node);
void lightAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData);

void test(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData);

void materialCreated(MObject& node, void *clientData = nullptr);
void materialAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData);
void materialChange(MObject &node);
void meshy(MObject& node);