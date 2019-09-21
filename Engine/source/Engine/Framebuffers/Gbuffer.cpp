#include "stdafx.h"
#include "Gbuffer.h"

#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/EnvironmentMap.h>
#include <Engine/GraphicsHelper/PbrPrefilter.h>
#include <Engine/SceneGraph/AbstractScene.h>
#include <Engine/Components/LightComponent.h>
#include <Engine/Prefabs/Skybox.h>


Gbuffer::Gbuffer(bool demo):
	FrameBuffer(demo?
		"Shaders/PostBufferDisplay.glsl":
		"Shaders/PostDeferredComposite.glsl", 
		GL_FLOAT, 2)
{
	m_CaptureDepth = true;
}

void Gbuffer::AccessShaderAttributes()
{
	//m_pShader->Upload("texGBufferA"_hash, 0);
	m_pShader->Upload("texGBufferB"_hash, 1);
	m_pShader->Upload("texGBufferC"_hash, 2);
}

void Gbuffer::UploadDerivedVariables()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//for position reconstruction
	//m_pShader->Upload("projectionA"_hash, CAMERA->GetDepthProjA());
	//m_pShader->Upload("projectionB"_hash, CAMERA->GetDepthProjB());
	m_pShader->Upload("viewProjInv"_hash, CAMERA->GetStatViewProjInv());
	//m_pShader->Upload("camPos"_hash, CAMERA->GetTransform()->GetPosition());

	if (SCENE->SkyboxEnabled())
	{
		m_pShader->Upload("texIrradiance"_hash, 3);
		api->LazyBindTexture(3, GL_TEXTURE_CUBE_MAP, SCENE->GetEnvironmentMap()->GetIrradianceHandle());

		m_pShader->Upload("texEnvRadiance"_hash, 4);
		api->LazyBindTexture(4, GL_TEXTURE_CUBE_MAP, SCENE->GetEnvironmentMap()->GetRadianceHandle());

		m_pShader->Upload("MAX_REFLECTION_LOD"_hash, static_cast<float>(SCENE->GetEnvironmentMap()->GetNumMipMaps()));
	}

	m_pShader->Upload("texBRDFLUT"_hash, 5);
	TextureData* pLUT = PbrPrefilter::GetInstance()->GetLUT();
	api->LazyBindTexture(5, pLUT->GetTarget(), pLUT->GetHandle());
}