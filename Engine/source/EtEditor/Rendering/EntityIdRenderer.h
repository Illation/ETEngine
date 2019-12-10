#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtRendering/GraphicsContext/Viewport.h>
#include <EtRendering/GraphicsContext/GraphicsTypes.h>


class AbstractScene;
class Camera;
namespace render {
	class Material;
}


//---------------------------------
// EntityIdRenderer
//
// Renders all entities to the scene using an ID as a color
//
class EntityIdRenderer final : public I_ViewportListener
{
public:	
	// construct destruct
	//--------------------
	EntityIdRenderer() = default;
	~EntityIdRenderer();

private:
	EntityIdRenderer(const EntityIdRenderer& t) = delete;
	EntityIdRenderer& operator=(const EntityIdRenderer& t) = delete;

public:
	void Initialize();

private:
	void CreateRenderTarget();
	void DestroyRenderTarget();

	// Functionality
	//---------------
public:
	void Pick(ivec2 const pixel, Viewport* const viewport, AbstractScene* const scene, std::function<void(Entity* const)>& onEntityPicked);
	
	// Viewport Listener interface
	//-----------------------------
protected:

	void OnViewportPreRender(T_FbLoc const targetFb) override;
	void OnViewportPostFlush(T_FbLoc const targetFb) override;

	// Utility
	//---------
private:
	void RecursiveDrawEntity(Entity* const entity, Camera const& camera) const;


	// Data
	///////

	AssetPtr<ShaderData> m_Shader;
	AssetPtr<render::Material> m_Material;

	T_FbLoc m_DrawTarget;
	TextureData* m_DrawTex = nullptr;
	T_RbLoc m_DrawDepth;

	ivec2 m_LastViewSize; // if the view size changes we update the render target on resize 

	ivec2 m_PixelToPick;
	Viewport* m_ViewportToPickFrom = nullptr;
	AbstractScene* m_Scene = nullptr;
	std::function<void(Entity* const)> m_OnEntityPicked;
};

