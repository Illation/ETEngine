#include "stdafx.hpp"

#include "LightVolume.hpp"
#include "../Game/Materials/LightMaterial.hpp"
#include "../Graphics/MeshFilter.hpp"
#include <glm/gtx/transform.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "../Game/Materials/NullMaterial.hpp"
#include "../Framebuffers/Gbuffer.hpp"
#include "../Graphics/ShaderData.hpp"
#include "../Graphics/TextureData.hpp"

PointLightVolume::PointLightVolume()
{

}
PointLightVolume::~PointLightVolume()
{
	SafeDelete(m_pMaterial);
	SafeDelete(m_pNullMaterial);
}

void PointLightVolume::Initialize()
{
	m_pMeshFilter = ContentManager::Load<MeshFilter>("Resources/Models/Sphere.dae");
	//Create the material

	m_pMaterial = new LightMaterial();
	m_pMaterial->Initialize();
	//m_pMeshFilter->BuildVertexBuffer(m_pMaterial);
	m_pNullMaterial = new NullMaterial();
	m_pNullMaterial->Initialize();
	//m_pMeshFilter->BuildVertexBuffer(m_pNullMaterial);

	IsInitialized = true;
}
void PointLightVolume::Draw(glm::vec3 pos, float radius, glm::vec3 col)
{
	//Make sure everything is set up
	if (!IsInitialized)
	{
		Initialize();
	}
	glm::mat4 World = glm::translate(pos)*glm::scale(glm::vec3(radius));

	//Draw the null material in the stencil buffer
	//glEnable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
	//glClear(GL_STENCIL_BUFFER_BIT);
	//glStencilFunc(GL_ALWAYS, 0, 0);
	//glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	//glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

	//auto vO = m_pMeshFilter->GetVertexObject(m_pNullMaterial);
	//glBindVertexArray(vO.array);
	//m_pNullMaterial->UploadVariables(World);
	//glDrawElementsInstanced(GL_TRIANGLES, m_pMeshFilter->GetIndexCount(), GL_UNSIGNED_INT, 0, 1);

	//Draw the Light material on the gbuffer
	//glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	//glDisable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendEquation(GL_FUNC_ADD);
	//glBlendFunc(GL_ONE, GL_ONE);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);

	auto vO = m_pMeshFilter->GetVertexObject(m_pMaterial);//changing the vertex array might not be necessary
	glBindVertexArray(vO.array);
	m_pMaterial->SetLight(pos, col, radius);
	m_pMaterial->UploadVariables(World);
	glDrawElementsInstanced(GL_TRIANGLES, m_pMeshFilter->GetIndexCount(), GL_UNSIGNED_INT, 0, 1);

	//glCullFace(GL_BACK);
	//glDisable(GL_BLEND);
}

DirectLightVolume::DirectLightVolume(){}
DirectLightVolume::~DirectLightVolume() 
{
	glDeleteBuffers(1, &m_QuadVBO);
	glDeleteVertexArrays(1, &m_QuadVAO);
}
void DirectLightVolume::Initialize()
{
	m_pShader = ContentManager::Load<ShaderData>("Shaders/FwdLightDirectionalShader.glsl");

	m_uCol = glGetUniformLocation(m_pShader->GetProgram(), "Color");
	m_uDir = glGetUniformLocation(m_pShader->GetProgram(), "Direction");
	m_uCamPos = glGetUniformLocation(m_pShader->GetProgram(), "camPos");

	GLfloat quadVertices[] = {
		//Change winding because lights get drawn with backface culling enabled
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	// Setup plane VAO
	glGenVertexArrays(1, &m_QuadVAO);
	glGenBuffers(1, &m_QuadVBO);
	glBindVertexArray(m_QuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	
	IsInitialized = true;
}
void DirectLightVolume::Draw(glm::vec3 dir, glm::vec3 col)
{
	if (!IsInitialized) Initialize();

	glBindVertexArray(m_QuadVAO);
	glUseProgram(m_pShader->GetProgram());

	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texPosAO"), 0);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texNormMetSpec"), 1);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texBaseColRough"), 2);
	auto gbufferTex = SCENE->GetGBuffer()->GetTextures();
	for (size_t i = 0; i < gbufferTex.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, gbufferTex[i]->GetHandle());
	}

	glUniform3f(m_uDir, dir.x, dir.y, dir.z);
	glUniform3f(m_uCol, col.x, col.y, col.z);
	glm::vec3 cPos = CAMERA->GetTransform()->GetPosition();
	glUniform3f(m_uCamPos, cPos.x, cPos.y, cPos.z);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}