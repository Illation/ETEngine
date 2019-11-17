#pragma once
#include <Engine/Graphics/Material.h>


//---------------------------------
// ColorMaterial
//
// Simple material that draws meshes in a color, but doesn't set the variables itself - variables should be directly uploaded on its underlying shader
//
class ColorMaterial : public Material
{
public:
	ColorMaterial();
	virtual ~ColorMaterial() = default;

private:
	void LoadTextures() override {}
	void AccessShaderAttributes() override {}
	void UploadDerivedVariables() override {}
};

