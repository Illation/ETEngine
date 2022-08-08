#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtRHI/GraphicsContext/Viewport.h>
#include <EtRHI/GraphicsContext/GraphicsTypes.h>

#include <EtRendering/MaterialSystem/MaterialData.h>

#include <EtFramework/ECS/EntityFwd.h>


namespace et { namespace render {
	class Camera;
} }


namespace et {
namespace edit {


//---------------------------------
// EntityIdRenderer
//
// Renders all entities to the scene using an ID as a color
//
class EntityIdRenderer final
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
	void Pick(ivec2 const pixel, rhi::Viewport* const viewport, std::function<void(fw::T_EntityId const)>& onEntityPicked);

	// Utility
	//---------
private:
	void OnViewportPreRender(rhi::T_FbLoc const targetFb);
	void OnViewportPostFlush(rhi::T_FbLoc const targetFb);

	void DrawEntity(fw::T_EntityId const entity, render::Camera const& camera) const;
	void GetIdColor(fw::T_EntityId const id, vec4& col) const;


	// Data
	///////

	AssetPtr<rhi::ShaderData> m_Shader;
	AssetPtr<render::Material> m_Material;

	rhi::T_FbLoc m_DrawTarget;
	rhi::TextureData* m_DrawTex = nullptr;
	rhi::T_RbLoc m_DrawDepth;

	ivec2 m_LastViewSize; // if the view size changes we update the render target on resize 

	ivec2 m_PixelToPick;
	rhi::Viewport* m_ViewportToPickFrom = nullptr;
	rhi::T_ViewportEventCallbackId m_VPCallbackId = rhi::T_ViewportEventDispatcher::INVALID_ID;
	std::function<void(fw::T_EntityId const)> m_OnEntityPicked;
};


} // namespace edit
} // namespace et
