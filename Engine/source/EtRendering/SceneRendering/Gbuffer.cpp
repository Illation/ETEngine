#include "stdafx.h"
#include "Gbuffer.h"

#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/GraphicsTypes/TextureData.h>

#include <EtRendering/GraphicsTypes/EnvironmentMap.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/SceneStructure/RenderScene.h>


namespace et {
namespace render {


Gbuffer::Gbuffer():
	FrameBuffer("Shaders/PostDeferredComposite.glsl", 2)
{
	m_CaptureDepth = true;
}

void Gbuffer::UploadDerivedVariables()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	//for position reconstruction
	render::ShadedSceneRenderer const* const sceneRenderer = render::ShadedSceneRenderer::GetCurrent();

	AssetPtr<EnvironmentMap> envMap = sceneRenderer->GetScene()->GetSkybox().m_EnvironmentMap;
	if (envMap != nullptr)
	{
		m_Shader->Upload("uTexIrradiance"_hash, envMap->GetIrradiance());
		m_Shader->Upload("uTexRadiance"_hash, envMap->GetRadiance());
		m_Shader->Upload("uMaxReflectionLod"_hash, static_cast<float>(envMap->GetNumMipMaps()));
	}

	rhi::TextureData const* const lut = RenderingSystems::Instance()->GetPbrPrefilter().GetLUT();
	m_Shader->Upload("uTexBrdfLut"_hash, lut);

	//m_Shader->Upload("uTexGBufferA"_hash, GetTextures()[0]);
	m_Shader->Upload("uTexGBufferB"_hash, static_cast<rhi::TextureData const*>(GetTextures()[1]));
	m_Shader->Upload("uTexGBufferC"_hash, static_cast<rhi::TextureData const*>(GetTextures()[2]));
}


} // namespace render
} // namespace et
