#pragma once
#include "ContextRenderTarget.h"

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
	void RenderContext(render::T_FbLoc const targetFb, 
		ContextRenderTarget& renderTarget, 
		Context& contexts, 
		render::E_PolygonMode const polyMode = render::E_PolygonMode::Fill);

	void RenderWorldContext(render::T_FbLoc const targetFb, 
		ContextRenderTarget& renderTarget, 
		Context& contexts, 
		mat4 const& transform, 
		vec4 const& color,
		bool const enableDepth,
		render::E_PolygonMode const polyMode = render::E_PolygonMode::Fill);

	// utility
	//---------
private:
	void SetupContextRendering(render::I_GraphicsContextApi* const api, ContextRenderTarget &renderTarget);

	// Data
	///////

	RefPtr<RmlGlobal> m_RmlGlobal;
	AssetPtr<render::ShaderData> m_GenericShader;
	AssetPtr<render::ShaderData> m_RmlSdfShader;
	AssetPtr<render::ShaderData> m_RmlBlitShader;
	AssetPtr<render::ShaderData> m_RmlBlit3DShader;
};


} // namespace gui
} // namespace et
