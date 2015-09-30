#pragma once
#include "maya_includes.h"

void meshAttributeChanged(MNodeMessage::AttributeMessage p_Msg, MPlug &p_Plug, MPlug &p_Plug2, void *p_ClientData);
void nodeCreated(MObject &node, void *clientData);