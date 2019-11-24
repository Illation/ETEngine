#pragma once
#include <EtCore/Content/AssetPointer.h>


class EnvironmentMap;


namespace render {


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
