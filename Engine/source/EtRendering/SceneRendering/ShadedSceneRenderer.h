#pragma once
#include "ShadowRenderer.h"
#include "Gbuffer.h"
#include "ScreenSpaceReflections.h"
#include "PostProcessingRenderer.h"
#include "SceneRendererFwd.h"

#include <EtRendering/GraphicsTypes/Camera.h>
#include <EtRendering/GraphicsContext/ViewportRenderer.h>
#include <EtRendering/Extensions/RenderEvents.h>
#include <EtRendering/Extensions/DebugRenderer.h>


namespace et {
namespace render {


class Scene;
class MaterialCollection;


//---------------------------------
// ShadedSceneRenderer
//
// Renders a Scene to the viewport
//
class ShadedSceneRenderer final : public I_ViewportRenderer, public I_ShadowRenderer
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

	// functionality
	//---------------
	void SetCamera(core::T_SlotId const cameraId) { m_CameraId = cameraId; }
	void SetRenderMode(E_RenderMode const mode) { m_RenderMode = mode; }

	// Viewport Renderer Interface
	//-----------------------------
protected:
	rttr::type GetType() const override { return rttr::type::get<ShadedSceneRenderer>(); }
	void OnInit() override {}
	void OnDeinit() override {}
	void OnResize(ivec2 const dim) override;
	void OnRender(T_FbLoc const targetFb) override;

	// Shadow Renderer Interface
	//-----------------------------
public:
	Camera const& GetCamera() const override;
	void DrawShadow(I_Material const* const nullMaterial) override;

	// accessors
	//--------------
public:
	Gbuffer& GetGBuffer() { return m_GBuffer; }
	Gbuffer const& GetGBuffer() const { return m_GBuffer; }

	render::Scene const* GetScene() const { return m_RenderScene; }

	T_RenderEventDispatcher& GetEventDispatcher() { return m_Events; }

	E_PolygonMode Get3DPolyMode() const;

	// utility
	//---------
private:
	PostProcessingSettings const& GetPostProcessingSettings();
	void DrawMaterialCollectionGroup(core::slot_map<MaterialCollection> const& collectionGroup);

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	void DrawDebugVisualizations();
#endif


	// Data
	///////

	bool m_IsInitialized = false;

	// scene rendering
	vec3 m_ClearColor;
	ivec2 m_Dimensions;

	E_RenderMode m_RenderMode = E_RenderMode::Shaded;
	PostProcessingSettings m_OverrideSettings;

	core::T_SlotId m_CameraId = core::INVALID_SLOT_ID;

	render::Scene* m_RenderScene = nullptr;

	ShadowRenderer m_ShadowRenderer;
	Gbuffer m_GBuffer;
	ScreenSpaceReflections m_SSR;
	PostProcessingRenderer m_PostProcessing;

	AssetPtr<ShaderData> m_SkyboxShader;

	T_RenderEventDispatcher m_Events;

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	DebugRenderer m_DebugRenderer;
#endif
};


} // namespace render
} // namespace et
