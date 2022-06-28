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


	// Data
	///////

private:
	RefPtr<RmlGlobal> m_RmlGlobal;
	AssetPtr<render::ShaderData> m_RmlShader;
	AssetPtr<render::ShaderData> m_RmlSdfShader;
	AssetPtr<render::ShaderData> m_RmlBlitShader;
};


} // namespace gui
} // namespace et
