#pragma once
#include "OurMaterial.h"

OurMaterial::OurMaterial()
{
	color = Vector4(0, 0, 0, 1);
	diffuseTexFilePath = "";
	texture = nullptr;
}

OurMaterial::~OurMaterial()
{

}

Vector4 OurMaterial::getColor() const
{
	return color;
}
const char* OurMaterial::getDiffuseTexFilePath() const
{
	return diffuseTexFilePath.c_str();
}

Texture::Sampler* OurMaterial::getTexure() const
{
	return texture;
}