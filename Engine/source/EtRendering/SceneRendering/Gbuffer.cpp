#include "stdafx.h"
#include "Gbuffer.h"

#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/EnvironmentMap.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/GraphicsHelper/RenderScene.h>


Gbuffer::Gbuffer(bool demo):
	FrameBuffer(demo ? "Shaders/PostBufferDisplay.glsl" : "Shaders/PostDeferredComposite.glsl", E_DataType::Float, 2)
{
	m_CaptureDepth = true;
}

void Gbuffer::AccessShaderAttributes()
{
	m_pShader->Upload("texGBufferB"_hash, 1);
	m_pShader->Upload("texGBufferC"_hash, 2);
}

void Gbuffer::UploadDerivedVariables()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//for position reconstruction
	render::ShadedSceneRenderer const* const sceneRenderer = render::ShadedSceneRenderer::GetCurrent();
	m_pShader->Upload("viewProjInv"_hash, sceneRenderer->GetCamera().GetStatViewProjInv());

	AssetPtr<EnvironmentMap> envMap = sceneRenderer->GetScene()->GetSkybox().m_EnvironmentMap;
	if (envMap != nullptr)
	{
		m_pShader->Upload("texIrradiance"_hash, 3);
		api->LazyBindTexture(3, E_TextureType::CubeMap, envMap->GetIrradianceHandle());

		m_pShader->Upload("texEnvRadiance"_hash, 4);
		api->LazyBindTexture(4, E_TextureType::CubeMap, envMap->GetRadianceHandle());

		m_pShader->Upload("MAX_REFLECTION_LOD"_hash, static_cast<float>(envMap->GetNumMipMaps()));
	}

	m_pShader->Upload("texBRDFLUT"_hash, 5);
	TextureData* pLUT = RenderingSystems::Instance()->GetPbrPrefilter().GetLUT();
	api->LazyBindTexture(5, pLUT->GetTargetType(), pLUT->GetHandle());
}