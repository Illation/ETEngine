#pragma once
#include "ShadowRenderer.h"
#include "Gbuffer.h"
#include "ScreenSpaceReflections.h"
#include "PostProcessingRenderer.h"

#include <Engine/Graphics/Camera.h>
#include <Engine/GraphicsContext/ViewportRenderer.h>


namespace render {


class Scene;


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

	// accessors
	//--------------
public:
	Camera& GetCamera() { return m_Camera; }

	Gbuffer& GetGBuffer() { return m_GBuffer; }
	Gbuffer const& GetGBuffer() const { return m_GBuffer; }

	render::Scene const* GetScene() const { return m_RenderScene; }

	// Data
	///////
private:

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
};


} // namespace render
