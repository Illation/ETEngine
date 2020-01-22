#pragma once
#include "ShadowRendererInterface.h"

#include <EtCore/Content/AssetPointer.h>


namespace et {
namespace render {


// forward
class ShaderData;
class DirectionalShadowData;


//---------------------------------
// ShadowRenderer
//
// Class that can fill out shadow data for light sources
//
class ShadowRenderer final
{
	// construct destruct
	//---------------------
public:
	ShadowRenderer() = default;
	~ShadowRenderer() = default;

	void Initialize();

	// functionality
	//---------------
	void MapDirectional(mat4 const& lightTransform, DirectionalShadowData& shadowData, I_ShadowRenderer* const shadowRenderer);

	// Data
	///////
private:

	AssetPtr<ShaderData> m_Shader;
};


} // namespace render
} // namespace et
