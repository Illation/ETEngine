#include "stdafx.h"
#include "Planet.h"

#include "Patch.h"
#include "Atmosphere.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/Shader.h>


namespace et {
namespace render {


//===============
// Planet Params
//===============
	

RTTR_REGISTRATION
{
	rttr::registration::class_<PlanetParams>("planet params")
		.property("radius", &PlanetParams::radius)
		.property("height", &PlanetParams::height)
		.property("diffuse texture", &PlanetParams::texDiffuseId)
		.property("detail texture 1", &PlanetParams::texDetail1Id)
		.property("detail texture 2", &PlanetParams::texDetail2Id)
		.property("height texture", &PlanetParams::texHeightId)
		.property("height detail texture", &PlanetParams::texHeightDetailId);
}


//========
// Planet
//========


//-------------------------
// Planet::Init
//
void Planet::Init(PlanetParams const& params, core::T_SlotId const nodeId)
{
	m_Node = nodeId;

	m_Radius = params.radius;
	m_MaxHeight = params.height;

	m_TexDiffuse = core::ResourceManager::Instance()->GetAssetData<TextureData>(params.texDiffuseId);
	m_TexDetail1 = core::ResourceManager::Instance()->GetAssetData<TextureData>(params.texDetail1Id);
	m_TexDetail2 = core::ResourceManager::Instance()->GetAssetData<TextureData>(params.texDetail2Id);

	m_TexHeight = core::ResourceManager::Instance()->GetAssetData<TextureData>(params.texHeightId);
	m_TexHeightDetail = core::ResourceManager::Instance()->GetAssetData<TextureData>(params.texHeightDetailId);

	m_Triangulator.Init(this);
}


} // namespace render
} // namespace et
