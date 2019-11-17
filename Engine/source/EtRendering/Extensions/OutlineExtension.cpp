#include "stdafx.h"
#include "OutlineExtension.h"

#include <EtRendering/GraphicsTypes/Material.h>
#include <EtRendering/GraphicsTypes/Mesh.h>
#include <EtRendering/Materials/ColorMaterial.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>



//====================
// Outline Extension
//====================


//---------------------------------
// OutlineExtension::AddMesh
//
void OutlineExtension::AddMesh(AssetPtr<MeshData> const mesh, render::T_NodeId const node)
{
	// try finding an outline list with the current color
	auto listIt = std::find_if(m_Lists.begin(), m_Lists.end(), [this](OutlineList const& list)
		{
			return etm::nearEqualsV(list.color, m_Color);
		});

	// create a new one if none was found
	if (listIt == m_Lists.cend())
	{
		m_Lists.emplace_back(OutlineList());
		listIt = std::prev(m_Lists.end());
		listIt->color = m_Color;
	}

	// get the VAO of our mesh associated with the color material
	T_ArrayLoc const vao = mesh->GetSurface(RenderingSystems::Instance()->GetColorMaterial())->GetVertexArray();

	// try finding an existing mesh with the VAO
	auto foundMeshIt = std::find_if(listIt->meshes.begin(), listIt->meshes.end(), [vao](render::MaterialCollection::Mesh const& meshInst)
		{
			return meshInst.m_VAO == vao;
		});

	// if none was found, create that meshes draw data
	if (foundMeshIt == listIt->meshes.cend())
	{
		listIt->meshes.emplace_back(render::MaterialCollection::Mesh());
		foundMeshIt = std::prev(listIt->meshes.end());

		foundMeshIt->m_VAO = vao;
		foundMeshIt->m_IndexCount = static_cast<uint32>(mesh->GetIndexCount());
		foundMeshIt->m_IndexDataType = mesh->GetIndexDataType();
		foundMeshIt->m_BoundingVolume = mesh->GetBoundingSphere();
	}

	// add an instance
	foundMeshIt->m_Instances.emplace_back(node);
}

