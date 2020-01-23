#include "stdafx.h"
#include "Planet.h"

#include "Patch.h"
#include "Atmosphere.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/Shader.h>


namespace et {
namespace render {


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
