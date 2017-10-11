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
#include "../Helper/MathHelper.hpp"
#include <gtx/transform.hpp>
#include <gtx/quaternion.hpp>
#include <gtx/matrix_decompose.hpp>
#include <gtx/euler_angles.hpp>
#include "../Graphics/Frustum.hpp"

Atmosphere::Atmosphere() 
{
}
Atmosphere::~Atmosphere()
{
}

void Atmosphere::Precalculate()
{
	//Calculate look up textures here
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
	glm::vec3 pos = pPlanet->GetTransform()->GetPosition();
	float surfaceRadius = pPlanet->GetRadius();
	radius += surfaceRadius;
	float icoRadius = radius / 0.996407747f;//scale up the sphere so the face center reaches the top of the atmosphere

	Sphere objSphere = Sphere(pos, radius);
	if (CAMERA->GetFrustum()->ContainsSphere(objSphere) == VolumeCheck::OUTSIDE)
		return;
																		
	glm::mat4 World = glm::translate(pos)*glm::scale(glm::vec3(icoRadius));

	STATE->SetShader(m_pShader);

	glUniformMatrix4fv(m_uMatModel, 1, GL_FALSE, glm::value_ptr(World));
	glUniformMatrix4fv(m_uMatWVP, 1, GL_FALSE, glm::value_ptr(CAMERA->GetViewProj()));

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
	glUniformMatrix4fv(m_uViewProjInv, 1, GL_FALSE, glm::value_ptr(CAMERA->GetStatViewProjInv()));
	glUniform3fv(m_uCamPos, 1, glm::value_ptr(CAMERA->GetTransform()->GetPosition()));

	glUniform3fv(m_uPosition, 1, glm::value_ptr(pos));
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