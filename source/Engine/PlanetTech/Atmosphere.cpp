#include "stdafx.hpp"
#include "Atmosphere.hpp"

#include "Planet.hpp"
#include "../Framebuffers/Gbuffer.hpp"

#include "../Graphics/ShaderData.hpp"
#include "../SceneGraph/AbstractScene.hpp"
#include "../Components/LightComponent.hpp"
#include "../Graphics/TextureData.hpp"
#include "../GraphicsHelper/PrimitiveRenderer.hpp"
#include "../GraphicsHelper/RenderPipeline.hpp"
#include "../Graphics/Frustum.hpp"

Atmosphere::Atmosphere() 
{
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
	AtmoPreComputer::GetInstance()->Precalculate( this );
}

void Atmosphere::Initialize()
{
	Precalculate();
	//Load and compile Shaders
	m_pShader = ContentManager::Load<ShaderData>("Shaders/PostAtmosphere.glsl");
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

	STATE->SetShader(m_pShader);

	glUniformMatrix4fv(m_uMatModel, 1, GL_FALSE, etm::valuePtr(World));
	glUniformMatrix4fv(m_uMatWVP, 1, GL_FALSE, etm::valuePtr(CAMERA->GetViewProj()));

	// #todo: stop repeating this everywhere
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferA"), 0);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferB"), 1);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferC"), 2);
	auto gbufferTex = RenderPipeline::GetInstance()->GetGBuffer()->GetTextures();
	for (size_t i = 0; i < gbufferTex.size(); i++)
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


AtmoPreComputer::AtmoPreComputer()
{
	Init();
}
AtmoPreComputer::~AtmoPreComputer()
{
	Unload();
}

void AtmoPreComputer::Init()
{
	//General texture format for atmosphere
	m_TexParams = TextureParameters();
	m_TexParams.wrapS = GL_CLAMP_TO_EDGE;
	m_TexParams.wrapT = GL_CLAMP_TO_EDGE;
	m_TexParams.wrapR = GL_CLAMP_TO_EDGE;

	m_ScatteringTexDim = ivec3(
		Atmosphere::INSCATTER_NU * Atmosphere::INSCATTER_MU_S,
		Atmosphere::INSCATTER_MU,
		Atmosphere::INSCATTER_R);

	m_pComputeTransmittance = ContentManager::Load<ShaderData>("Shaders/AtmoPreComp/ComputeTransmittance.glsl");
	m_pComputeDirectIrradiance = ContentManager::Load<ShaderData>("Shaders/AtmoPreComp/ComputeDirectIrradiance.glsl");
	m_pComputeSingleScattering = ContentManager::Load<ShaderData>("Shaders/AtmoPreComp/ComputeSingleScattering.glsl");
	m_pComputeScatteringDensity = ContentManager::Load<ShaderData>("Shaders/AtmoPreComp/ComputeScatteringDensity.glsl");
	m_pComputeIndirectIrradiance = ContentManager::Load<ShaderData>("Shaders/AtmoPreComp/ComputeIndirectIrradiance.glsl");
	m_pComputeMultipleScattering = ContentManager::Load<ShaderData>("Shaders/AtmoPreComp/ComputeMultipleScattering.glsl");



	m_IsInitialized = true;
}

void AtmoPreComputer::Unload()
{
	m_IsInitialized = false;
}

void AtmoPreComputer::Precalculate( Atmosphere* atmo )
{
	if (!m_IsInitialized)
	{
		Init();
	}

	GLenum format = GL_RGBA;
	GLenum internalFormat2D = GL_RGBA32F;
	GLenum internalFormat3D = GL_RGBA16F;//half precision

	//Specific texture initialization
	atmo->m_TexTransmittance = new TextureData(Atmosphere::TRANSMITTANCE_W, Atmosphere::TRANSMITTANCE_H, 
		internalFormat2D, format, GL_FLOAT);
	atmo->m_TexTransmittance->Build();
	atmo->m_TexTransmittance->SetParameters(m_TexParams);
	atmo->m_TexIrradiance = new TextureData(Atmosphere::IRRADIANCE_W, Atmosphere::IRRADIANCE_H, 
		internalFormat2D, format, GL_FLOAT);
	atmo->m_TexIrradiance->Build();
	atmo->m_TexIrradiance->SetParameters(m_TexParams);

	atmo->m_TexInscatter = new TextureData(m_ScatteringTexDim.x, m_ScatteringTexDim.y,
		internalFormat3D, format, GL_FLOAT, m_ScatteringTexDim.z);
	atmo->m_TexInscatter->Build();
	atmo->m_TexInscatter->SetParameters(m_TexParams);
}
