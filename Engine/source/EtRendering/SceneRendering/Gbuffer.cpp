#include "stdafx.h"
#include "Gbuffer.h"

#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/EnvironmentMap.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/SceneStructure/RenderScene.h>


Gbuffer::Gbuffer(bool demo):
	FrameBuffer(demo ? "Shaders/PostBufferDisplay.glsl" : "Shaders/PostDeferredComposite.glsl", E_DataType::Float, 2)
{
	m_CaptureDepth = true;
}

void Gbuffer::AccessShaderAttributes()
{
}

void Gbuffer::UploadDerivedVariables()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//for position reconstruction
	render::ShadedSceneRenderer const* const sceneRenderer = render::ShadedSceneRenderer::GetCurrent();

	AssetPtr<EnvironmentMap> envMap = sceneRenderer->GetScene()->GetSkybox().m_EnvironmentMap;
	if (envMap != nullptr)
	{
		m_pShader->Upload("texIrradiance"_hash, envMap->GetIrradiance());
		m_pShader->Upload("texEnvRadiance"_hash, envMap->GetRadiance());

		m_pShader->Upload("MAX_REFLECTION_LOD"_hash, static_cast<float>(envMap->GetNumMipMaps()));
	}

	TextureData const* lut = RenderingSystems::Instance()->GetPbrPrefilter().GetLUT();
	m_pShader->Upload("texBRDFLUT"_hash, lut);
}