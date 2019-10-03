#pragma once
#include <Engine/Graphics/Material.h>


//---------------------------------
// IdMaterial
//
// Simple material that draws meshes in a color, but doesn't set the variables itself - variables should be directly uploaded on its underlying shader
//
class IdMaterial : public Material
{
public:
	IdMaterial();
	virtual ~IdMaterial() = default;

private:
	void LoadTextures() override {}
	void AccessShaderAttributes() override {}
	void UploadDerivedVariables() override {}
};

