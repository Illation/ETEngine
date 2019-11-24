#pragma once
#include <EtRendering/GraphicsTypes/Material.h>


class NullMaterial : public Material
{
public:
	NullMaterial();
	virtual ~NullMaterial() = default;

private:
	void LoadTextures() override {}
	void AccessShaderAttributes() override {}
	void UploadDerivedVariables() override {}
};

