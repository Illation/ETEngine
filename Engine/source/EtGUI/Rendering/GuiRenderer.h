#pragma once
#include "TextRenderer.h"
#include "SpriteRenderer.h"

#include <EtGUI/GuiExtension.h>

#include <EtRendering/Extensions/RenderEvents.h>


namespace et {
namespace gui {


class RmlGlobal;


//---------------------------------
// GuiRenderer
//
// Renders RmlUI contexts
//
class GuiRenderer final
{
public:
	// construct destruct
	//--------------------
	GuiRenderer() = default;

	void Init();
	void Deinit();

	// functionality
	//---------------
	void RenderContexts(render::Viewport const* const viewport, render::T_FbLoc const targetFb, Context* const contexts, size_t const count);

	// utility
	//---------
	void GenerateFramebuffer(ivec2 const dim);
	void DeleteFramebuffer();

	// Data
	///////

private:
	RefPtr<RmlGlobal> m_RmlGlobal;
	AssetPtr<render::ShaderData> m_RmlShader;
	AssetPtr<render::ShaderData> m_RmlSdfShader;
	AssetPtr<render::ShaderData> m_RmlBlitShader;

	// target
	render::T_FbLoc m_RmlFb;
	render::T_RbLoc m_RmlRbo;
	UniquePtr<render::TextureData> m_RmlTex;
};


} // namespace gui
} // namespace et
