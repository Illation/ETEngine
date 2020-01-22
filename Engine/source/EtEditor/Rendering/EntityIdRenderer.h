#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtRendering/GraphicsContext/Viewport.h>
#include <EtRendering/GraphicsContext/GraphicsTypes.h>

#include <EtFramework/ECS/EntityFwd.h>


namespace et { namespace render {
	class Camera;
	class Material;
} }


namespace et {
namespace edit {


//---------------------------------
// EntityIdRenderer
//
// Renders all entities to the scene using an ID as a color
//
class EntityIdRenderer final : public render::I_ViewportListener
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
	void Pick(ivec2 const pixel, render::Viewport* const viewport, std::function<void(fw::T_EntityId const)>& onEntityPicked);
	
	// Viewport Listener interface
	//-----------------------------
protected:

	void OnViewportPreRender(render::T_FbLoc const targetFb) override;
	void OnViewportPostFlush(render::T_FbLoc const targetFb) override;

	// Utility
	//---------
private:
	void DrawEntity(fw::T_EntityId const entity, render::Camera const& camera) const;
	void GetIdColor(fw::T_EntityId const id, vec4& col) const;


	// Data
	///////

	AssetPtr<render::ShaderData> m_Shader;
	AssetPtr<render::Material> m_Material;

	render::T_FbLoc m_DrawTarget;
	render::TextureData* m_DrawTex = nullptr;
	render::T_RbLoc m_DrawDepth;

	ivec2 m_LastViewSize; // if the view size changes we update the render target on resize 

	ivec2 m_PixelToPick;
	render::Viewport* m_ViewportToPickFrom = nullptr;
	std::function<void(fw::T_EntityId const)> m_OnEntityPicked;
};


} // namespace edit
} // namespace et
