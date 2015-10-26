#pragma once
#include "gameplay.h"
#include "Texture.h"

using namespace gameplay;

class OurMaterial
{
public:
	Vector4 diffuseColor;
	Vector3 ambientColor;
	std::string diffuseTexFilePath;
	Texture::Sampler* texture;

	OurMaterial();

	~OurMaterial();

	Vector4 getDiffuseColor() const;

	Vector3 getAmbientColor() const;

	const char* getDiffuseTexFilePath() const;

	Texture::Sampler* getTexure() const;
	
};