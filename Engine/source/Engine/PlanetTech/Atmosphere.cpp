#include "stdafx.h"
#include "Atmosphere.h"

#include "Planet.h"
#include "AtmospherePrecompute.h"

#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Helper/Commands.h>

#include <Engine/Framebuffers/Gbuffer.h>
#include <Engine/Components/LightComponent.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Frustum.h>
#include <Engine/Graphics/Light.h>
#include <Engine/GraphicsHelper/SpriteRenderer.h>
#include <Engine/GraphicsHelper/PrimitiveRenderer.h>
#include <Engine/GraphicsHelper/SceneRenderer.h>
#include <Engine/Prefabs/Skybox.h>
#include <Engine/SceneGraph/AbstractScene.h>


Atmosphere::Atmosphere(T_Hash const parameterAssetId)
{
	m_Params = AtmosphereParameters(parameterAssetId, m_SkyColor, m_SunColor);
}
Atmosphere::~Atmosphere()
{
	delete m_TexTransmittance;
	m_TexTransmittance = nullptr;
	delete m_TexIrradiance;
	m_TexIrradiance = nullptr;
	delete m_TexInscatter;
	m_TexInscatter = nullptr;
}

void Atmosphere::Precalculate()
{
	//Calculate look up textures here
	AtmospherePrecompute::GetInstance()->Precalculate( this );
}

void Atmosphere::Initialize()
{
	Precalculate();
	//Load and compile Shaders
	m_pShader = ResourceManager::Instance()->GetAssetData<ShaderData>("PostAtmosphere.glsl"_hash);
}
void Atmosphere::Draw(Planet* pPlanet, float radius)
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	vec3 pos = pPlanet->GetTransform()->GetPosition();
	float surfaceRadius = pPlanet->GetRadius();
	radius += surfaceRadius;
	float icoRadius = radius / 0.996407747f;//scale up the sphere so the face center reaches the top of the atmosphere

	Sphere objSphere = Sphere(pos, radius);
	if (CAMERA->GetFrustum()->ContainsSphere(objSphere) == VolumeCheck::OUTSIDE)
		return;

	//mat4 World = etm::translate(pos)*etm::scale(vec3(icoRadius));
	mat4 World = etm::scale(vec3(icoRadius))*etm::translate(pos);

	//Hotreload shader
	//if (INPUT->GetKeyState(E_KbdKey::LeftAlt) == E_KeyState::Down &&
	//	INPUT->GetKeyState(E_KbdKey::R) == E_KeyState::Pressed)
	//{
	//	//if there is a debugger attached copy over the resource files 
	//	DebugCopyResourceFiles();
	//	//reload the shader
	//	m_pShader = ContentManager::Reload<ShaderData>("Shaders/PostAtmosphere.glsl");
	//}
	api->SetShader(m_pShader.get());

	m_pShader->Upload("model"_hash, World);
	m_pShader->Upload("worldViewProj"_hash, CAMERA->GetViewProj());

	// #todo: stop repeating this everywhere
	m_pShader->Upload("texGBufferA"_hash, 0);
	//m_pShader->Upload("texGBufferB"_hash, 1);
	//m_pShader->Upload("texGBufferC"_hash, 2);
	auto gbufferTex = SceneRenderer::GetInstance()->GetGBuffer()->GetTextures();
	for (uint32 i = 0; i < (uint32)gbufferTex.size(); i++)
	{
		api->LazyBindTexture(i, E_TextureType::Texture2D, gbufferTex[i]->GetHandle());
	}
	m_pShader->Upload("projectionA"_hash, CAMERA->GetDepthProjA());
	m_pShader->Upload("projectionB"_hash, CAMERA->GetDepthProjB());
	m_pShader->Upload("viewProjInv"_hash, CAMERA->GetStatViewProjInv());
	m_pShader->Upload("camPos"_hash, CAMERA->GetTransform()->GetPosition());

	m_pShader->Upload("Position"_hash, pos);
	m_pShader->Upload("Radius"_hash, radius);
	//m_pShader->Upload("SurfaceRadius"_hash, surfaceRadius);

	m_Params.Upload(m_pShader.get(), "uAtmosphere");
	AtmospherePrecompute::GetInstance()->GetSettings().UploadTextureSize(m_pShader.get());

	//m_pShader->Upload("uSkySpectralRadToLum"_hash, etm::vecCast<float>(m_SkyColor));
	//m_pShader->Upload("uSunSpectralRadToLum"_hash, etm::vecCast<float>(m_SunColor));

	m_pShader->Upload("uTexIrridiance"_hash, 3);
	api->LazyBindTexture(3, E_TextureType::Texture2D, m_TexIrradiance->GetHandle());
	m_pShader->Upload("uTexInscatter"_hash, 4);
	api->LazyBindTexture(4, E_TextureType::Texture3D, m_TexInscatter->GetHandle());
	m_pShader->Upload("uTexTransmittance"_hash, 5);
	api->LazyBindTexture(5, E_TextureType::Texture2D, m_TexTransmittance->GetHandle());

	m_pShader->Upload("SunDir"_hash, m_pSun->GetTransform()->GetForward());
	m_pShader->Upload("uSunSize"_hash, vec2(tan(m_Params.sun_angular_radius), cos(m_Params.sun_angular_radius)));
	//DirectionalLight* pDirLight = m_pSun->GetLight<DirectionalLight>();
	//if (pDirLight)
	//{
	//	m_pShader->Upload("SunIntensity"_hash, pDirLight->GetBrightness());
	//}

	api->SetCullEnabled(true);
	api->SetFaceCullingMode(E_FaceCullMode::Front);
	api->SetDepthEnabled(false);
	api->SetBlendEnabled(true);
	api->SetBlendEquation(E_BlendEquation::Add);
	api->SetBlendFunction(E_BlendFactor::One, E_BlendFactor::One);
	PrimitiveRenderer::GetInstance()->Draw<primitives::IcoSphere<3> >();
	api->SetFaceCullingMode(E_FaceCullMode::Back);
	api->SetBlendEnabled(false);
	api->SetDepthEnabled(true);
	api->SetCullEnabled(false);
}