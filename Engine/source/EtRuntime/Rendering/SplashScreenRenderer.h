#pragma once
#include <EtRendering/GraphicsContext/ViewportRenderer.h>

#include <EtGUI/Rendering/SpriteRenderer.h>
#include <EtGUI/Rendering/TextRenderer.h>


// fwd
namespace et { namespace render {
	class TextureData;
} }


namespace et {
namespace rt {


//---------------------------------
// SplashScreenRenderer
//
// Renders a texture and overlay text, useful while transitioning scenes
//
class SplashScreenRenderer final : public render::I_ViewportRenderer
{
	// construct destruct
	//--------------------
public:
	SplashScreenRenderer() : render::I_ViewportRenderer() {}
	~SplashScreenRenderer() = default;

	// functionality
	//---------------
	void Init();
	void Deinit();

	// Viewport Renderer Interface
	//-----------------------------
protected:
	rttr::type GetType() const override { return rttr::type::get<SplashScreenRenderer>(); }
	void OnInit() override {}
	void OnDeinit() override {}
	void OnResize(ivec2 const dim) override;
	void OnRender(render::T_FbLoc const targetFb) override;


	// Data
	///////

private:
	bool m_IsInitialized = false;

	std::string m_Title;
	std::string m_Subtitle;

	AssetPtr<render::TextureData> m_SplashBackgroundTex;
	AssetPtr<gui::SdfFont> m_SplashTitleFont;
	AssetPtr<gui::SdfFont> m_SplashRegFont;

	// scene rendering
	ivec2 m_Dimensions;

	gui::TextRenderer m_TextRenderer;
	gui::SpriteRenderer m_SpriteRenderer;
};


} // namespace rt
} // namespace et
