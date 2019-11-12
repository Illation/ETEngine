#pragma once
#include "PostProcessingRenderer.h"
#include "Gbuffer.h"

#include <Engine/Graphics/Camera.h>
#include <Engine/GraphicsContext/ViewportRenderer.h>


namespace render {


class Scene;


//---------------------------------
// ShadedSceneRenderer
//
// Renders a Scene to the viewport
//
class ShadedSceneRenderer final : public I_ViewportRenderer
{
public:
	// construct destruct
	//--------------------
	ShadedSceneRenderer(render::Scene const* const renderScene);
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

	// accessors
	//--------------
public:
	Camera& GetCamera() { return m_Camera; }
	Gbuffer* GetGBuffer() { return &m_GBuffer; }

	// Data
	///////
private:

	bool m_IsInitialized = false;

	// scene rendering
	vec3 m_ClearColor;
	ivec2 m_Dimensions;

	Camera m_Camera;

	render::Scene const* m_RenderScene = nullptr;
	T_FbLoc m_TargetFb = 0u;

	Gbuffer m_GBuffer;
	PostProcessingRenderer m_PostProcessing;
};


} // namespace render
