#include "stdafx.hpp"
#include "Atmosphere.hpp"

#include "Planet.h"
#include "../Framebuffers/Gbuffer.hpp"

#include "../Graphics/ShaderData.hpp"
#include "../SceneGraph/AbstractScene.hpp"
#include "../Components/LightComponent.hpp"
#include "../Graphics/TextureData.hpp"
#include "../GraphicsHelper/PrimitiveRenderer.hpp"
#include "../GraphicsHelper/RenderPipeline.hpp"
#include "../Helper/MathHelper.hpp"
#include <glm/gtx/transform.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "../Graphics/Frustum.h"

Atmosphere::Atmosphere(Planet* pPlanet) 
	: m_pPanet(pPlanet)
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
	//Load and compile Shaders
	m_pShader = ContentManager::Load<ShaderData>("Shaders/PostAtmosphere.glsl");
	glUseProgram(m_pShader->GetProgram());

	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferA"), 0);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferB"), 1);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferC"), 2);

	m_uCamPos = glGetUniformLocation(m_pShader->GetProgram(), "camPos");
	m_uProjA = glGetUniformLocation(m_pShader->GetProgram(), "projectionA");
	m_uProjB = glGetUniformLocation(m_pShader->GetProgram(), "projectionB");
	m_uViewProjInv = glGetUniformLocation(m_pShader->GetProgram(), "viewProjInv");

	m_uPosition = glGetUniformLocation(m_pShader->GetProgram(), "Position");
	m_uRadius = glGetUniformLocation(m_pShader->GetProgram(), "Radius");
}
void Atmosphere::Draw(glm::vec3 pos, float radius)
{
	Sphere objSphere = Sphere(pos, radius);
	if (CAMERA->GetFrustum()->ContainsSphere(objSphere) == VolumeCheck::OUTSIDE)
		return;

	glm::mat4 World = glm::translate(pos)*glm::scale(glm::vec3(radius));

	glUseProgram(m_pShader->GetProgram());

	// #todo: stop repeating this everywhere
	auto gbufferTex = RenderPipeline::GetInstance()->GetGBuffer()->GetTextures();
	for (size_t i = 0; i < gbufferTex.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, gbufferTex[i]->GetHandle());
	}
	glUniform1f(m_uProjA, CAMERA->GetDepthProjA());
	glUniform1f(m_uProjB, CAMERA->GetDepthProjB());
	glUniformMatrix4fv(m_uViewProjInv, 1, GL_FALSE, glm::value_ptr(CAMERA->GetStatViewProjInv()));
	glUniform3fv(m_uCamPos, 1, glm::value_ptr(CAMERA->GetTransform()->GetPosition()));

	glUniform3fv(m_uPosition, 1, glm::value_ptr(pos));
	glUniform1f(m_uRadius, radius);

	STATE->SetCullEnabled(true);
	STATE->SetFaceCullingMode(GL_FRONT);
	STATE->SetDepthEnabled(false);
	PrimitiveRenderer::GetInstance()->Draw<primitives::IcoSphere<3> >();
}