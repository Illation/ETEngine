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
	void RenderContext(rhi::T_FbLoc const targetFb, 
		ContextRenderTarget& renderTarget, 
		Context& contexts, 
		rhi::E_PolygonMode const polyMode = rhi::E_PolygonMode::Fill);

	void RenderWorldContext(rhi::T_FbLoc const targetFb, 
		ContextRenderTarget& renderTarget, 
		Context& contexts, 
		mat4 const& transform, 
		vec4 const& color,
		bool const enableDepth,
		rhi::E_PolygonMode const polyMode = rhi::E_PolygonMode::Fill);

	// utility
	//---------
private:
	void SetupContextRendering(rhi::I_GraphicsContextApi* const api, ContextRenderTarget &renderTarget);

	// Data
	///////

	RefPtr<RmlGlobal> m_RmlGlobal;
	AssetPtr<rhi::ShaderData> m_GenericShader;
	AssetPtr<rhi::ShaderData> m_RmlSdfShader;
	AssetPtr<rhi::ShaderData> m_RmlBlitShader;
	AssetPtr<rhi::ShaderData> m_RmlBlit3DShader;
};


} // namespace gui
} // namespace et
