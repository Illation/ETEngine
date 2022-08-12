#pragma once
#include <EtRHI/GraphicsContext/ViewportRenderer.h>

#include <EtGUI/Context/Context.h>
#include <EtGUI/Rendering/GuiRenderer.h>


namespace et {
namespace rt {


//---------------------------------
// SplashScreenRenderer
//
// Renders a texture and overlay text, useful while transitioning scenes
//
class SplashScreenRenderer final : public rhi::I_ViewportRenderer
{
	// construct destruct
	//--------------------
public:
	SplashScreenRenderer() : rhi::I_ViewportRenderer() {}
	~SplashScreenRenderer() = default;

	// functionality
	//---------------
	void Init();
	void Deinit();

	void SetGuiDocument(core::HashString const documentId);

	// Viewport Renderer Interface
	//-----------------------------
protected:
	rttr::type GetType() const override { return rttr::type::get<SplashScreenRenderer>(); }
	void OnInit() override {}
	void OnDeinit() override {}
	void OnResize(ivec2 const dim) override;
	void OnRender(rhi::T_FbLoc const targetFb) override;


	// Data
	///////

private:
	bool m_IsInitialized = false;

	gui::Context m_GuiContext;
	gui::ContextRenderTarget m_ContextRenderTarget;
	gui::GuiRenderer m_GuiRenderer;
};


} // namespace rt
} // namespace et
