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
		m_pShader->Upload("uTexIrradiance"_hash, envMap->GetIrradiance());
		m_pShader->Upload("uTexRadiance"_hash, envMap->GetRadiance());
		m_pShader->Upload("uMaxReflectionLod"_hash, static_cast<float>(envMap->GetNumMipMaps()));
	}

	rhi::TextureData const* const lut = RenderingSystems::Instance()->GetPbrPrefilter().GetLUT();
	m_pShader->Upload("uTexBrdfLut"_hash, lut);
}


} // namespace render
} // namespace et
