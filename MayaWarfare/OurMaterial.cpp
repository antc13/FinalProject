#pragma once
#include "OurMaterial.h"

OurMaterial::OurMaterial()
{
	diffuseColor = Vector4(0, 0, 0, 1);
	ambientColor = Vector3(0, 0, 0);
	diffuseTexFilePath = "";
	texture = nullptr;
}

OurMaterial::~OurMaterial()
{

}

Vector4 OurMaterial::getDiffuseColor() const
{
	return diffuseColor;
}

Vector3 OurMaterial::getAmbientColor() const
{
	return ambientColor;
}

const char* OurMaterial::getDiffuseTexFilePath() const
{
	return diffuseTexFilePath.c_str();
}

Texture::Sampler* OurMaterial::getTexure() const
{
	return texture;
}