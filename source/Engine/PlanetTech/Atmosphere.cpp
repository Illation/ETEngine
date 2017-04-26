#include "stdafx.hpp"
#include "Atmosphere.hpp"

#include "Planet.h"
#include "../Framebuffers/Gbuffer.hpp"

#include "../Graphics/ShaderData.hpp"
#include "../SceneGraph/AbstractScene.hpp"
#include "../Components/LightComponent.hpp"
#include "../Graphics/TextureData.hpp"

Atmosphere::Atmosphere(Planet* pPlanet) 
	: FrameBuffer("Shaders/PostDeferredComposite.glsl", GL_FLOAT, 1)
	, m_pPanet(pPlanet)
{
}
Atmosphere::~Atmosphere()
{
}

void Atmosphere::Precalculate()
{
	//Calculate look up textures here
}

void Atmosphere::AccessShaderAttributes()
{
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferA"), 0);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferB"), 1);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferC"), 2);

	m_uCamPos = glGetUniformLocation(m_pShader->GetProgram(), "camPos");
	m_uProjA = glGetUniformLocation(m_pShader->GetProgram(), "projectionA");
	m_uProjB = glGetUniformLocation(m_pShader->GetProgram(), "projectionB");
	m_uViewProjInv = glGetUniformLocation(m_pShader->GetProgram(), "viewProjInv");
}
void Atmosphere::UploadDerivedVariables()
{
	// #todo: stop repeating this everywhere
	auto gbufferTex = SCENE->GetGBuffer()->GetTextures();
	for (size_t i = 0; i < gbufferTex.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, gbufferTex[i]->GetHandle());
	}
	glUniform1f(m_uProjA, CAMERA->GetDepthProjA());
	glUniform1f(m_uProjB, CAMERA->GetDepthProjB());
	glUniformMatrix4fv(m_uViewProjInv, 1, GL_FALSE, glm::value_ptr(CAMERA->GetStatViewProjInv()));
	glUniform3fv(m_uCamPos, 1, glm::value_ptr(CAMERA->GetTransform()->GetPosition()));
}