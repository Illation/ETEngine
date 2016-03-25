#include "stdafx.hpp"
#include "Gbuffer.hpp"

#include "../Graphics/ShaderData.hpp"
#include "../SceneGraph/AbstractScene.hpp"
#include "../Components/LightComponent.hpp"
#include "../Graphics/TextureData.hpp"
#include "../Prefabs/Skybox.hpp"

Gbuffer::Gbuffer(bool demo):
	FrameBuffer(demo?
		"Resources/Shaders/PostBufferDisplay.glsl":
		"Resources/Shaders/PostDeferredComposite.glsl", 
		GL_FLOAT, 3)
{
}
Gbuffer::~Gbuffer()
{
}

void Gbuffer::AccessShaderAttributes()
{
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texPosAO"), 0);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texNormMetSpec"), 1);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texBaseColRough"), 2);

	m_uCamPos = glGetUniformLocation(m_pShader->GetProgram(), "camPos");
}
void Gbuffer::UploadDerivedVariables()
{
	auto lightVec = SCENE->GetLights();
	for (size_t i = 0; i < lightVec.size(); i++)
	{
		lightVec[i]->UploadVariables(m_pShader->GetProgram(), i);
	}

	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texEnvironment"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, SCENE->GetEnvironmentMap()->GetHandle());

	glm::vec3 cPos = CAMERA->GetTransform()->GetPosition();
	glUniform3f(m_uCamPos, cPos.x, cPos.y, cPos.z);
}