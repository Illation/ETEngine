#include "stdafx.h"
#include "Atmosphere.h"

#include "Planet.h"
#include "AtmospherePrecompute.h"

#include <EtCore/Helper/Commands.h>

#include <Engine/Framebuffers/Gbuffer.h>
#include <Engine/Components/LightComponent.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Frustum.h>
#include <Engine/Graphics/Light.h>
#include <Engine/GraphicsHelper/SpriteRenderer.h>
#include <Engine/GraphicsHelper/PrimitiveRenderer.h>
#include <Engine/GraphicsHelper/RenderPipeline.h>
#include <Engine/Prefabs/Skybox.h>
#include <Engine/SceneGraph/AbstractScene.h>


Atmosphere::Atmosphere(const std::string &paramFileName)
{
	m_Params = AtmosphereParameters(paramFileName, m_SkyColor, m_SunColor);
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
	m_pShader = ContentManager::Load<ShaderData>("Shaders/PostAtmosphere.glsl");
	GetUniforms();
}
void Atmosphere::Draw(Planet* pPlanet, float radius)
{
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
	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_LALT)) == E_KeyState::Down &&
		INPUT->GetKeyState(static_cast<uint32>(SDLK_r)) == E_KeyState::Pressed)
	{
		//if there is a debugger attached copy over the resource files 
		DebugCopyResourceFiles();
		//reload the shader
		m_pShader = ContentManager::Reload<ShaderData>("Shaders/PostAtmosphere.glsl");
		GetUniforms();
	}
	STATE->SetShader(m_pShader);

	glUniformMatrix4fv(m_uMatModel, 1, GL_FALSE, etm::valuePtr(World));
	glUniformMatrix4fv(m_uMatWVP, 1, GL_FALSE, etm::valuePtr(CAMERA->GetViewProj()));

	// #todo: stop repeating this everywhere
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferA"), 0);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferB"), 1);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferC"), 2);
	auto gbufferTex = RenderPipeline::GetInstance()->GetGBuffer()->GetTextures();
	for (uint32 i = 0; i < (uint32)gbufferTex.size(); i++)
	{
		STATE->LazyBindTexture(i, GL_TEXTURE_2D, gbufferTex[i]->GetHandle());
	}
	glUniform1f(m_uProjA, CAMERA->GetDepthProjA());
	glUniform1f(m_uProjB, CAMERA->GetDepthProjB());
	glUniformMatrix4fv(m_uViewProjInv, 1, GL_FALSE, etm::valuePtr(CAMERA->GetStatViewProjInv()));
	glUniform3fv(m_uCamPos, 1, etm::valuePtr(CAMERA->GetTransform()->GetPosition()));

	glUniform3fv(m_uPosition, 1, etm::valuePtr(pos));
	glUniform1f(m_uRadius, radius);
	glUniform1f(m_uSurfaceRadius, surfaceRadius);

	m_Params.Upload(m_pShader, "uAtmosphere");
	AtmospherePrecompute::GetInstance()->GetSettings().UploadTextureSize(m_pShader);

	vec3 skySpectralRadToLum = vec3((float)m_SkyColor.x, (float)m_SkyColor.y, (float)m_SkyColor.z);
	glUniform3fv(m_uSkySpectralRadToLum, 1, etm::valuePtr(skySpectralRadToLum));
	vec3 sunSpectralRadToLum = vec3((float)m_SunColor.x, (float)m_SunColor.y, (float)m_SunColor.z);
	glUniform3fv(m_uSunSpectralRadToLum, 1, etm::valuePtr(sunSpectralRadToLum));

	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "uTexIrridiance"), 3);
	STATE->LazyBindTexture(3, GL_TEXTURE_2D, m_TexIrradiance->GetHandle());
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "uTexInscatter"), 4);
	STATE->LazyBindTexture(4, GL_TEXTURE_3D, m_TexInscatter->GetHandle());
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "uTexTransmittance"), 5);
	STATE->LazyBindTexture(5, GL_TEXTURE_2D, m_TexTransmittance->GetHandle());

	glUniform3fv(m_uSunDir, 1, etm::valuePtr(m_pSun->GetTransform()->GetForward()));
	DirectionalLight* pDirLight = m_pSun->GetLight<DirectionalLight>();
	if (pDirLight) glUniform1f(m_uSunIntensity, pDirLight->GetBrightness());
	vec2 sunSize = vec2(tan(m_Params.sun_angular_radius), cos(m_Params.sun_angular_radius));
	glUniform2fv(m_uSunSize, 1, etm::valuePtr(sunSize));

	STATE->SetCullEnabled(true);
	STATE->SetFaceCullingMode(GL_FRONT);
	STATE->SetDepthEnabled(false);
	STATE->SetBlendEnabled(true);
	STATE->SetBlendEquation(GL_FUNC_ADD);
	STATE->SetBlendFunction(GL_ONE, GL_ONE);
	PrimitiveRenderer::GetInstance()->Draw<primitives::IcoSphere<3> >();
	STATE->SetFaceCullingMode(GL_BACK);
	STATE->SetBlendEnabled(false);
	STATE->SetDepthEnabled(true);
	STATE->SetCullEnabled(false);
}

void Atmosphere::GetUniforms()
{
	if (!m_pShader)
		return;

	STATE->SetShader(m_pShader);

	m_uMatModel = glGetUniformLocation(m_pShader->GetProgram(), "model");
	m_uMatWVP = glGetUniformLocation(m_pShader->GetProgram(), "worldViewProj");

	m_uCamPos = glGetUniformLocation(m_pShader->GetProgram(), "camPos");
	m_uProjA = glGetUniformLocation(m_pShader->GetProgram(), "projectionA");
	m_uProjB = glGetUniformLocation(m_pShader->GetProgram(), "projectionB");
	m_uViewProjInv = glGetUniformLocation(m_pShader->GetProgram(), "viewProjInv");

	m_uPosition = glGetUniformLocation(m_pShader->GetProgram(), "Position");
	m_uRadius = glGetUniformLocation(m_pShader->GetProgram(), "Radius");
	m_uSurfaceRadius = glGetUniformLocation(m_pShader->GetProgram(), "SurfaceRadius");

	m_uSunDir = glGetUniformLocation(m_pShader->GetProgram(), "SunDir");
	m_uSunIntensity = glGetUniformLocation(m_pShader->GetProgram(), "SunIntensity");
	m_uSunSize = glGetUniformLocation(m_pShader->GetProgram(), "uSunSize");

	m_uSkySpectralRadToLum = glGetUniformLocation(m_pShader->GetProgram(), "uSkySpectralRadToLum");
	m_uSunSpectralRadToLum = glGetUniformLocation(m_pShader->GetProgram(), "uSunSpectralRadToLum");
}