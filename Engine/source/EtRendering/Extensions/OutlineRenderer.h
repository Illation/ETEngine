#pragma once
#include "OutlineExtension.h"
#include "RenderEvents.h"


namespace et {
namespace render {


class Camera;
class rhi::TextureData;
class Gbuffer;


//---------------------------------
// OutlineRenderer
//
// Rendering class that can draw outlines around 3D objects in the scene
//
class OutlineRenderer final
{
	// construct destruct
	//--------------------
public:
	OutlineRenderer() = default;
	~OutlineRenderer();

	void Init(Ptr<render::T_RenderEventDispatcher> const eventDispatcher);
	void Deinit();

	// Functionality
	//---------------
public:
	void OnWindowResize();
	void Draw(rhi::T_FbLoc const targetFb, OutlineExtension const& outlines, core::slot_map<mat4> const& nodes, Camera const& cam, Gbuffer const& gbuffer);

	// utility
	//---------
private:
	void CreateRenderTarget();
	void DestroyRenderTarget();

	// Data
	///////

	bool m_IsInitialized = false;

	rhi::T_FbLoc m_DrawTarget;
	rhi::TextureData* m_DrawTex = nullptr;
	rhi::T_RbLoc m_DrawDepth;

	AssetPtr<rhi::ShaderData> m_SobelShader;

	Ptr<render::T_RenderEventDispatcher> m_EventDispatcher;
	render::T_RenderEventCallbackId m_CallbackId = render::T_RenderEventDispatcher::INVALID_ID;

	rhi::T_ViewportEventCallbackId m_VPCallbackId = rhi::T_ViewportEventDispatcher::INVALID_ID;
};


} // namespace render
} // namespace et
