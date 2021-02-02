#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtRendering/GraphicsTypes/EnvironmentMap.h>


namespace et {
namespace render {


//----------------------
// Skybox
//
// Data required to draw a skybox in a scene
//
struct Skybox
{
	float m_Roughness = 0.2f;
	AssetPtr<EnvironmentMap> m_EnvironmentMap;
};


} // namespace render
} // namespace et
