#include "stdafx.h"
#include "OutlineRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>


//====================
// Outline Renderer
//====================


//---------------------------------
// SpriteRenderer::Initialize
//
void OutlineRenderer::Initialize()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	m_Shader = ResourceManager::Instance()->GetAssetData<ShaderData>("DebugRenderer.glsl"_hash);

	// init frame buffers
}

//---------------------------------
// SpriteRenderer::AddEntity
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
// SpriteRenderer::AddEntities
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
// SpriteRenderer::Draw
//
void OutlineRenderer::Draw(T_FbLoc const targetFb)
{
	if (m_Lists.empty())
	{
		return;
	}

	UNUSED(targetFb);

	m_Lists.clear();
}

//---------------------------------
// SpriteRenderer::AccessEntityListIt
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
