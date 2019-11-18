#include "stdafx.h"
#include "DirectionalShadowData.h"

#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


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

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//Calculate cascade distances
	float distMult = graphicsSettings.CSMDrawDistance / powf(2.f, static_cast<float>(graphicsSettings.NumCascades - 1));

	m_Cascades.clear();
	for (int32 cascadeIdx = 0; cascadeIdx < graphicsSettings.NumCascades; ++cascadeIdx)
	{
		CascadeData cascade;

		cascade.distance = static_cast<float>((cascadeIdx + 1) ^ 2) * distMult;

		// Create depth texture
		cascade.texture = new TextureData(resolution, E_ColorFormat::Depth, E_ColorFormat::Depth, E_DataType::Float);
		cascade.texture->Build();

		TextureParameters params(false, true);
		params.wrapS = E_TextureWrapMode::ClampToEdge;
		params.wrapT = E_TextureWrapMode::ClampToEdge;
		cascade.texture->SetParameters(params);

		// create render target
		api->GenFramebuffers(1, &(cascade.fbo));
		api->BindFramebuffer(cascade.fbo);
		api->LinkTextureToFboDepth(cascade.texture->GetHandle());
		//only depth components
		api->SetDrawBufferCount(0);
		api->SetReadBufferEnabled(false);

		api->BindFramebuffer(0);

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
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	for (CascadeData& cascade : m_Cascades)
	{
		api->DeleteFramebuffers(1, &(cascade.fbo));
		SafeDelete(cascade.texture);
	}
}


} // namespace render
