#include "stdafx.h"
#include "Gbuffer.h"

#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/TextureData.h>
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
Gbuffer::~Gbuffer()
{
}

void Gbuffer::AccessShaderAttributes()
{
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferA"), 0);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferB"), 1);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferC"), 2);

	m_uCamPos = glGetUniformLocation(m_pShader->GetProgram(), "camPos");
	m_uProjA = glGetUniformLocation(m_pShader->GetProgram(), "projectionA");
	m_uProjB = glGetUniformLocation(m_pShader->GetProgram(), "projectionB");
	m_uViewProjInv = glGetUniformLocation(m_pShader->GetProgram(), "viewProjInv");
}
void Gbuffer::UploadDerivedVariables()
{
	//for position reconstruction
	glUniform1f(m_uProjA, CAMERA->GetDepthProjA());
	glUniform1f(m_uProjB, CAMERA->GetDepthProjB());
	glUniformMatrix4fv(m_uViewProjInv, 1, GL_FALSE, etm::valuePtr(CAMERA->GetStatViewProjInv()));
	glUniform3fv(m_uCamPos, 1, etm::valuePtr(CAMERA->GetTransform()->GetPosition()));

	if (SCENE->SkyboxEnabled())
	{
		// #todo: stop setting textures and getting uniforms for the shader all the time
		glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texIrradiance"), 3);
		STATE->LazyBindTexture(3, GL_TEXTURE_CUBE_MAP, SCENE->GetEnvironmentMap()->GetIrradianceHandle());

		glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texEnvRadiance"), 4);
		STATE->LazyBindTexture(4, GL_TEXTURE_CUBE_MAP, SCENE->GetEnvironmentMap()->GetRadianceHandle());
		
		glUniform1f(glGetUniformLocation(m_pShader->GetProgram(), "MAX_REFLECTION_LOD"), (GLfloat)SCENE->GetEnvironmentMap()->GetNumMipMaps());
	}

	TextureData* pLUT = PbrPrefilter::GetInstance()->GetLUT();
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texBRDFLUT"), 5);
	STATE->LazyBindTexture(5, pLUT->GetTarget(), pLUT->GetHandle());
}