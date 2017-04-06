#include "stdafx.hpp"

#include "LightVolume.hpp"
#include "../Game/Materials/LightMaterial.hpp"
#include "../Graphics/MeshFilter.hpp"
#include <glm/gtx/transform.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "../Game/Materials/NullMaterial.hpp"

LightVolume::LightVolume()
{

}
LightVolume::~LightVolume()
{
	SafeDelete(m_pMaterial);
}

void LightVolume::Initialize()
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
void LightVolume::Draw(glm::vec3 pos, float radius, glm::vec3 col)
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