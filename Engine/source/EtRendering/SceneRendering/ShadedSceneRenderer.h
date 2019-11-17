#pragma once
#include "ShadowRenderer.h"
#include "Gbuffer.h"
#include "ScreenSpaceReflections.h"
#include "PostProcessingRenderer.h"
#include "OverlayRenderer.h"
#include "TextRenderer.h"
#include "SpriteRenderer.h"
#include "RenderEvents.h"

#include <Engine/Graphics/Camera.h>
#include <Engine/GraphicsContext/ViewportRenderer.h>


namespace render {


class Scene;


//---------------------------------
// ShadedSceneRenderer
//
// Renders a Scene to the viewport
//
class ShadedSceneRenderer final : public I_ViewportRenderer, public I_ShadowRenderer, public I_OverlayRenderer
{
	// GlobalAccess
	//---------------
public:
	static ShadedSceneRenderer* GetCurrent();

	// construct destruct
	//--------------------
	ShadedSceneRenderer(render::Scene* const renderScene);
	~ShadedSceneRenderer();

	void InitRenderingSystems();

	// Viewport Renderer Interface
	//-----------------------------
protected:
	std::type_info const& GetType() const override { return typeid(ShadedSceneRenderer); }
	void OnInit() override {}
	void OnDeinit() override {}
	void OnResize(ivec2 const dim) override;
	void OnRender(T_FbLoc const targetFb) override;

	// Shadow Renderer Interface
	//-----------------------------
public:
	void DrawShadow(NullMaterial* const nullMaterial) override;

	Camera const& GetCamera() const override { return m_Camera; }

	// Overlay Renderer Interface
	//-----------------------------
	void DrawOverlays(T_FbLoc const targetFb) override;

	// accessors
	//--------------
public:
	Camera& GetCamera() { return m_Camera; }

	Gbuffer& GetGBuffer() { return m_GBuffer; }
	Gbuffer const& GetGBuffer() const { return m_GBuffer; }

	render::Scene const* GetScene() const { return m_RenderScene; }

	TextRenderer& GetTextRenderer() { return m_TextRenderer; }
	SpriteRenderer& GetSpriteRenderer() { return m_SpriteRenderer; }

	RenderEventDispatcher& GetEventDispatcher() { return m_Events; }

	// utility
	//---------
private:
	void DrawMaterialCollectionGroup(core::slot_map<MaterialCollection> const& collectionGroup);

	// Data
	///////

	bool m_IsInitialized = false;

	// scene rendering
	vec3 m_ClearColor;
	ivec2 m_Dimensions;

	Camera m_Camera;

	render::Scene* m_RenderScene = nullptr;
	T_FbLoc m_TargetFb = 0u;

	ShadowRenderer m_ShadowRenderer;
	Gbuffer m_GBuffer;
	ScreenSpaceReflections m_SSR;
	PostProcessingRenderer m_PostProcessing;

	AssetPtr<ShaderData> m_SkyboxShader;

	TextRenderer m_TextRenderer;
	SpriteRenderer m_SpriteRenderer;

	RenderEventDispatcher m_Events;
};


} // namespace render
