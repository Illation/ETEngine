#pragma once
#include <EtCore/Content/AssetPointer.h>


namespace et {
namespace render {


class EnvironmentMap;


//----------------------
// Skybox
//
// Data required to draw a skybox in a scene
//
struct Skybox
{
	float m_Roughness = 0.15f;
	AssetPtr<EnvironmentMap> m_EnvironmentMap;
};


} // namespace render
} // namespace et
