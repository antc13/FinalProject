#pragma once
#include "gameplay.h"
#include "Texture.h"

using namespace gameplay;

class OurMaterial
{
public:
	Vector4 color;
	std::string diffuseTexFilePath;
	Texture::Sampler* texture;

	OurMaterial();

	~OurMaterial();

	Vector4 getColor() const;

	const char* getDiffuseTexFilePath() const;

	Texture::Sampler* getTexure() const;
	
};