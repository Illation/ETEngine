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
	void RenderContexts(render::T_FbLoc const targetFb, ContextRenderTarget& renderTarget, Context* const contexts, size_t const count);
	void RenderWorldContext(render::T_FbLoc const targetFb, 
		ContextRenderTarget& renderTarget, 
		Context& contexts, 
		mat4 const& transform, 
		bool const enableDepth);

	// utility
	//---------
private:
	void SetupContextRendering(render::I_GraphicsContextApi* const api, ContextRenderTarget &renderTarget);

	// Data
	///////

	RefPtr<RmlGlobal> m_RmlGlobal;
	AssetPtr<render::ShaderData> m_RmlShader;
	AssetPtr<render::ShaderData> m_RmlSdfShader;
	AssetPtr<render::ShaderData> m_RmlBlitShader;
	AssetPtr<render::ShaderData> m_RmlBlit3DShader;
};


} // namespace gui
} // namespace et
