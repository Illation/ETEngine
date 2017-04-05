#include "stdafx.hpp"

#include "LightVolume.hpp"
#include "../Game/Materials/LightMaterial.hpp"
#include "../Graphics/MeshFilter.hpp"
#include <glm/gtx/transform.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>

LightVolume::LightVolume()
{

}
LightVolume::~LightVolume()
{

}

void LightVolume::Initialize()
{
	m_pMeshFilter = ContentManager::Load<MeshFilter>("Resources/Models/Sphere.dae");
	//Create the material

	m_pMaterial = new LightMaterial();
	m_pMaterial->Initialize();
	//m_pMeshFilter->BuildVertexBuffer(m_pMaterial);

	IsInitialized = true;
}
void LightVolume::Draw(glm::vec3 pos, float radius)
{
	if (!IsInitialized)
	{
		Initialize();
	}
	auto vO = m_pMeshFilter->GetVertexObject(m_pMaterial);
	glBindVertexArray(vO.array);

	glm::mat4 World = glm::translate(pos)*glm::scale(glm::vec3(radius));
	m_pMaterial->UploadVariables(World);
	// Draw 
	glEnable(GL_DEPTH_TEST);
	glDrawElementsInstanced(GL_TRIANGLES, m_pMeshFilter->GetIndexCount(), GL_UNSIGNED_INT, 0, 1);
}