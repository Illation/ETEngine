#pragma once
#include "../../Graphics/Material.hpp"

class NullMaterial : public Material
{
public:
	NullMaterial();
	~NullMaterial();
private:
	void LoadTextures();
	void AccessShaderAttributes();

	void UploadDerivedVariables();

private:
};

