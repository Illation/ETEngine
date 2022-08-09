#include "stdafx.h"
#include "DirectionalShadowData.h"

#include <EtRHI/GraphicsTypes/TextureData.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace render {


//=========================
// Directional Shadow Data
//=========================


//---------------------------------
// DirectionalShadowData::Init
//
// Create the render target and texture
//
void DirectionalShadowData::Init(ivec2 const resolution)
{
	render::GraphicsSettings const& graphicsSettings = RenderingSystems::Instance()->GetGraphicsSettings();

	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	//Calculate cascade distances
	float distMult = graphicsSettings.CSMDrawDistance / powf(2.f, static_cast<float>(graphicsSettings.NumCascades - 1));

	m_Cascades.clear();
	for (int32 cascadeIdx = 0; cascadeIdx < graphicsSettings.NumCascades; ++cascadeIdx)
	{
		CascadeData cascade;

		cascade.distance = static_cast<float>((cascadeIdx + 1) ^ 2) * distMult;

		// Create depth texture
		cascade.texture = new rhi::TextureData(rhi::E_ColorFormat::Depth, resolution);
		cascade.texture->AllocateStorage();

		rhi::TextureParameters params(false, true);
		params.wrapS = rhi::E_TextureWrapMode::ClampToEdge;
		params.wrapT = rhi::E_TextureWrapMode::ClampToEdge;
		params.compareMode = rhi::E_TextureCompareMode::CompareRToTexture;
		cascade.texture->SetParameters(params);

		// create render target
		device->GenFramebuffers(1, &(cascade.fbo));
		device->BindFramebuffer(cascade.fbo);
		device->LinkTextureToFboDepth(cascade.texture->GetLocation());
		//only depth components
		device->SetDrawBufferCount(0);
		device->SetReadBufferEnabled(false);

		device->BindFramebuffer(0);

		m_Cascades.push_back(cascade);
	}
}

//---------------------------------
// DirectionalShadowData::Destroy
//
// This is not done in the d-tor, so that this type can be copied by value without freeing resources
//
void DirectionalShadowData::Destroy()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	for (CascadeData& cascade : m_Cascades)
	{
		device->DeleteFramebuffers(1, &(cascade.fbo));
		SafeDelete(cascade.texture);
	}
}


} // namespace render
} // namespace et
