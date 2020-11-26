#pragma once
#include "TextRenderer.h"
#include "SpriteRenderer.h"

#include <EtRendering/GraphicsContext/ViewportRenderer.h>


namespace et {
namespace render {


class TextureData;


//---------------------------------
// SplashScreenRenderer
//
// Renders a texture and overlay text, useful while transitioning scenes
//
class SplashScreenRenderer final : public I_ViewportRenderer
{
	// construct destruct
	//--------------------
public:
	SplashScreenRenderer() : I_ViewportRenderer() {}
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
	void OnRender(T_FbLoc const targetFb) override;

	// accessors
	//--------------
public:
	TextRenderer& GetTextRenderer() { return m_TextRenderer; }
	SpriteRenderer& GetSpriteRenderer() { return m_SpriteRenderer; }


	// Data
	///////

private:
	bool m_IsInitialized = false;

	std::string m_Title;
	std::string m_Subtitle;

	AssetPtr<TextureData> m_SplashBackgroundTex;
	AssetPtr<SpriteFont> m_SplashTitleFont;
	AssetPtr<SpriteFont> m_SplashRegFont;

	// scene rendering
	ivec2 m_Dimensions;

	TextRenderer m_TextRenderer;
	SpriteRenderer m_SpriteRenderer;
};


} // namespace render
} // namespace et
