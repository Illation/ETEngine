#include "stdafx.h"
#include "OutlineRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Materials/ColorMaterial.h>
#include <Engine/SceneGraph/Entity.h>


//====================
// Outline Renderer
//====================


//---------------------------------
// OutlineRenderer::d-tor
//
OutlineRenderer::~OutlineRenderer()
{
	SafeDelete(m_Material);
}

//---------------------------------
// OutlineRenderer::Initialize
//
void OutlineRenderer::Initialize()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	m_Shader = ResourceManager::Instance()->GetAssetData<ShaderData>("FwdColorShader.glsl"_hash);

	m_Material = new ColorMaterial();
	m_Material->Initialize();

	// init frame buffers
}

//---------------------------------
// OutlineRenderer::AddEntity
//
// Adds an entity to the current color list
//
void OutlineRenderer::AddEntity(Entity* const entity)
{
	auto listIt = AccessEntityListIt(m_Color);
	ET_ASSERT(listIt != m_Lists.cend());

	listIt->entities.emplace_back(entity);
}

//---------------------------------
// OutlineRenderer::AddEntities
//
// Add a bunch of entities to the current color list
//
void OutlineRenderer::AddEntities(std::vector<Entity*> const& entities)
{
	auto listIt = AccessEntityListIt(m_Color);
	ET_ASSERT(listIt != m_Lists.cend());

	listIt->entities.insert(listIt->entities.end(), entities.begin(), entities.end());
}

//---------------------------------
// OutlineRenderer::Draw
//
void OutlineRenderer::Draw(T_FbLoc const targetFb)
{
	if (m_Lists.empty())
	{
		return;
	}

	UNUSED(targetFb);

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->SetShader(m_Shader.get());
	m_Shader->Upload("worldViewProj"_hash, CAMERA->GetViewProj());

	for (EntityList& list : m_Lists)
	{
		m_Shader->Upload("uColor"_hash, m_Color);

		for (Entity* const entity : list.entities)
		{
			entity->RootDrawMaterial(static_cast<Material*>(m_Material));
		}
	}

	m_Lists.clear();
}

//---------------------------------
// OutlineRenderer::AccessEntityListIt
//
// Find or create an entity list matching our color
//
OutlineRenderer::T_EntityLists::iterator OutlineRenderer::AccessEntityListIt(vec4 const& col)
{
	auto listIt = std::find_if(m_Lists.begin(), m_Lists.end(), [&col](EntityList const& list)
		{
			return etm::nearEqualsV(list.color, col);
		});

	// create a new one if none was found
	if (listIt == m_Lists.cend())
	{
		m_Lists.emplace_back(EntityList());
		listIt = std::prev(m_Lists.end());
		listIt->color = col;
	}

	return listIt;
}
