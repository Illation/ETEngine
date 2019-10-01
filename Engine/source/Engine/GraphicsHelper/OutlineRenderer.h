#pragma once
#include <EtCore/Content/AssetPointer.h>


class ColorMaterial;


//---------------------------------
// OutlineRenderer
//
// Rendering class that can draw outlines around 3D objects in the scene
//
class OutlineRenderer final
{
private:
	// Definitions
	//--------------------------
	friend class SceneRenderer;

	//---------------------------------
	// OutlineRenderer::EntityList
	//
	// Lists that we can draw as a single unit without updating shader data (apart from model matricies)
	//
	struct EntityList
	{
		vec4 color;
		std::vector<Entity*> entities;
	};

	typedef std::vector<EntityList> T_EntityLists;

	// construct destruct
	//--------------------
	OutlineRenderer() = default;
	~OutlineRenderer();

	OutlineRenderer(const OutlineRenderer& t) = delete;
	OutlineRenderer& operator=(const OutlineRenderer& t) = delete;

	void Initialize();

	// Functionality
	//---------------
public:
	void SetColor(vec4 const& col) { m_Color = col; }
	void AddEntity(Entity* const entity);
	void AddEntities(std::vector<Entity*> const& entities);

private:
	void OnWindowResize() {}
	void Draw(T_FbLoc const targetFb);

	// utility
	//---------
	T_EntityLists::iterator AccessEntityListIt(vec4 const& col);

	// Data
	///////

	vec4 m_Color;
	T_EntityLists m_Lists;

	AssetPtr<ShaderData> m_Shader;
	ColorMaterial* m_Material;
};