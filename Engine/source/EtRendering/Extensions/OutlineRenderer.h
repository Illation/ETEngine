#pragma once
#include "OutlineExtension.h"
#include "RenderEvents.h"


class Camera;
class TextureData;
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

	void Init(render::T_RenderEventDispatcher* const eventDispatcher);
	void Deinit();

	// Functionality
	//---------------
public:
	void OnWindowResize();
	void Draw(T_FbLoc const targetFb, OutlineExtension const& outlines, core::slot_map<mat4> const& nodes, Camera const& cam, Gbuffer const& gbuffer);

	// utility
	//---------
private:
	void CreateRenderTarget();
	void DestroyRenderTarget();

	// Data
	///////

	bool m_IsInitialized = false;

	T_FbLoc m_DrawTarget;
	TextureData* m_DrawTex = nullptr;
	T_RbLoc m_DrawDepth;

	AssetPtr<ShaderData> m_SobelShader;

	render::T_RenderEventDispatcher* m_EventDispatcher = nullptr;
	render::T_RenderEventCallbackId m_CallbackId = render::T_RenderEventDispatcher::INVALID_ID;
};