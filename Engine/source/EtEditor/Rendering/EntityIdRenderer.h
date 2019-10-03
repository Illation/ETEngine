#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <Engine/GraphicsHelper/Viewport.h>
#include <Engine/GraphicsHelper/GraphicsTypes.h>


class IdMaterial;


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
	void Pick(ivec2 const pixel, Viewport* const viewport, std::function<void(Entity* const)>& onEntityPicked);
	
	// Viewport Listener interface
	//-----------------------------
protected:

	void OnViewportPreRender(T_FbLoc const targetFb) override;
	void OnViewportPostFlush(T_FbLoc const targetFb) override;

	// Data
	///////
private:

	AssetPtr<ShaderData> m_Shader;
	IdMaterial* m_Material;

	T_FbLoc m_DrawTarget;
	TextureData* m_DrawTex = nullptr;
	T_RbLoc m_DrawDepth;

	ivec2 m_LastViewSize; // if the view size changes we update the render target on resize 

	ivec2 m_PixelToPick;
	Viewport* m_ViewportToPickFrom = nullptr;
	std::function<void(Entity* const)> m_OnEntityPicked;
};

