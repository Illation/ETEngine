#include "stdafx.h"
#include "Patch.h"

#include "Triangulator.h"
#include "Planet.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/Frustum.h>


namespace et {
namespace render {


void Patch::Init(int16 const levels)
{
	m_Levels = levels;

	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	//Shader Init
	//***********
	m_pPatchShader = core::ResourceManager::Instance()->GetAssetData<ShaderData>(core::HashString("Shaders/PlanetPatch.glsl"));
	
	//Buffer Initialisation
	//*********************
	//Generate buffers and arrays
	m_VAO = api->CreateVertexArray();
	m_VBO = api->CreateBuffer();
	m_EBO = api->CreateBuffer();
	m_VBOInstance = api->CreateBuffer();
	//bind
	api->BindVertexArray(m_VAO);
	api->BindBuffer(E_BufferType::Vertex, m_VBO);
	//input layout
	//************
	//geometry
	api->SetVertexAttributeArrayEnabled(0, true);
	api->SetVertexAttributeArrayEnabled(1, true);
	api->DefineVertexAttributePointer(0, 2, E_DataType::Float, false, sizeof(PatchVertex), offsetof(PatchVertex, pos));
	api->DefineVertexAttributePointer(1, 2, E_DataType::Float, false, sizeof(PatchVertex), offsetof(PatchVertex, morph));
	//instances
	//bind
	api->BindBuffer(E_BufferType::Vertex, m_VBOInstance);
	api->SetVertexAttributeArrayEnabled(2, true);
	api->SetVertexAttributeArrayEnabled(3, true);
	api->SetVertexAttributeArrayEnabled(4, true);
	api->SetVertexAttributeArrayEnabled(5, true);
	api->DefineVertexAttribIPointer(2, 1, E_DataType::Int, sizeof(PatchInstance), offsetof(PatchInstance, level));
	api->DefineVertexAttributePointer(3, 3, E_DataType::Float, false, sizeof(PatchInstance), offsetof(PatchInstance, a));
	api->DefineVertexAttributePointer(4, 3, E_DataType::Float, false, sizeof(PatchInstance), offsetof(PatchInstance, r));
	api->DefineVertexAttributePointer(5, 3, E_DataType::Float, false, sizeof(PatchInstance), offsetof(PatchInstance, s));
	api->DefineVertexAttribDivisor(2, 1);
	api->DefineVertexAttribDivisor(3, 1);
	api->DefineVertexAttribDivisor(4, 1);
	api->DefineVertexAttribDivisor(5, 1);
	//Indices
	api->BindBuffer(E_BufferType::Index, m_EBO);
	//unbind
	api->BindBuffer(E_BufferType::Vertex, 0);
	api->BindVertexArray(0);

	GenerateGeometry(m_Levels);
}

void Patch::GenerateGeometry(int16 levels)
{
	//clear
	m_Vertices.clear();
	m_Indices.clear();
	//Generate
	m_Levels = levels;
	m_RC = 1 + (uint32)pow(2, (int32)m_Levels);

	float delta = 1 / (float)(m_RC-1);

	uint32 rowIdx = 0;
	uint32 nextIdx = 0;
	for (uint32 row = 0; row < m_RC; row++)
	{
		uint32 numCols = m_RC - row;
		nextIdx += numCols;
		for (uint32 column = 0; column < numCols; column++)
		{
			//calc position
			vec2 pos = vec2(column / (float)(m_RC-1), row / (float)(m_RC - 1));
			//calc morph
			vec2 morph = vec2(0, 0);
			if (row % 2 == 0)
			{
				if (column % 2 == 1) morph = vec2(-delta, 0);
			}
			else
			{
				if (column % 2 == 0) morph = vec2(0, delta);
				else morph = vec2(delta, -delta);
			}
			//create vertex
			m_Vertices.push_back(PatchVertex(pos, morph));
			//calc index
			if (row < m_RC - 1 && column < numCols - 1)
			{
				m_Indices.push_back(rowIdx + column);
				m_Indices.push_back(nextIdx + column);
				m_Indices.push_back(1 + rowIdx + column);
				if (column < numCols - 2)
				{
					m_Indices.push_back(nextIdx + column);
					m_Indices.push_back(1 + nextIdx + column);
					m_Indices.push_back(1 + rowIdx + column);
				}
			}
		}
		rowIdx = nextIdx;
	}

	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	// rebind
	api->BindBuffer(E_BufferType::Vertex, m_VBO);
	api->SetBufferData(E_BufferType::Vertex, m_Vertices.size() * sizeof(PatchVertex), m_Vertices.data(), E_UsageHint::Dynamic);
	api->BindBuffer(E_BufferType::Index, m_EBO);
	api->SetBufferData(E_BufferType::Index, sizeof(uint32)*m_Indices.size(), m_Indices.data(), E_UsageHint::Static);
	api->BindBuffer(E_BufferType::Vertex, 0);
}

void Patch::BindInstances(std::vector<PatchInstance> const& instances)
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	//update buffer
	m_NumInstances = (int32)instances.size();
	api->BindBuffer(E_BufferType::Vertex, m_VBOInstance);
	api->SetBufferData(E_BufferType::Vertex, instances.size() * sizeof(PatchInstance), instances.data(), E_UsageHint::Static);
	api->BindBuffer(E_BufferType::Vertex, 0);
}

void Patch::UploadDistanceLUT(std::vector<float> const& distances)
{
	ContextHolder::GetRenderContext()->SetShader(m_pPatchShader.get());
	for (size_t i = 0; i < distances.size(); i++)
	{
		m_pPatchShader->Upload(GetHash("distanceLUT[" + std::to_string(i) + "]"), distances[i]);
	}
}

void Patch::Draw(Planet const& planet, mat4 const& transform)
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	api->SetShader(m_pPatchShader.get());

	// Pass transformations to the shader
	m_pPatchShader->Upload("model"_hash, transform);

	//Set other uniforms here too!
	m_pPatchShader->Upload("uCamPos"_hash, planet.GetTriangulator().GetFrustum().GetPositionOS()); // can't use the shared camera because we need OS pos
	m_pPatchShader->Upload("radius"_hash, planet.GetRadius());
	m_pPatchShader->Upload("morphRange"_hash, m_MorphRange);

	m_pPatchShader->Upload("texDiffuse"_hash, planet.GetTexDiffuse());
	m_pPatchShader->Upload("texHeight"_hash, planet.GetTexHeight());
	m_pPatchShader->Upload("texDetail1"_hash, planet.GetTexDetail1());
	m_pPatchShader->Upload("texDetail2"_hash, planet.GetTexDetail2());
	m_pPatchShader->Upload("texHeightDetail"_hash, planet.GetTexHeightDetail());

	//Bind Object vertex array
	api->BindVertexArray(m_VAO);

	//Draw the object
	api->DrawElementsInstanced(E_DrawMode::Triangles, static_cast<uint32>(m_Indices.size()), E_DataType::UInt, 0, m_NumInstances);

	//unbind vertex array
	api->BindVertexArray(0);
}

Patch::~Patch()
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	api->DeleteVertexArray(m_VAO);
	api->DeleteBuffer(m_EBO);
	api->DeleteBuffer(m_VBO);
	api->DeleteBuffer(m_VBOInstance);
}


} // namespace render
} // namespace et
