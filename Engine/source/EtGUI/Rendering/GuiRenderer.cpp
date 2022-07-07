#include "stdafx.h"
#include "GuiRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>

#include <EtGUI/Context/RmlGlobal.h>


namespace et {
namespace gui {


//==============
// GUI Renderer
//==============


//----------------------------------
// GuiRenderer::Init
//
void GuiRenderer::Init()
{
	m_RmlGlobal = RmlGlobal::GetInstance(); // might initialize RML if this is the first GUI renderer
	m_GenericShader = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("Shaders/PostGenericUi.glsl"));
	m_RmlSdfShader = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("Shaders/PostRmlUiSdf.glsl"));
	m_RmlBlitShader = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("Shaders/PostRmlUiBlit.glsl"));
	m_RmlBlit3DShader = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("Shaders/PostRmlUiBlit3D.glsl"));
}

//----------------------------------
// GuiRenderer::Deinit
//
void GuiRenderer::Deinit()
{
	m_RmlGlobal = nullptr;

	m_GenericShader = nullptr;
	m_RmlBlitShader = nullptr;
	m_RmlBlit3DShader = nullptr;
	m_RmlSdfShader = nullptr;
}

//----------------------------------
// GuiRenderer::RenderContexts
//
// Render a list of contexts to the target framebuffer
//
void GuiRenderer::RenderContext(render::T_FbLoc const targetFb, ContextRenderTarget& renderTarget, Context& context)
{
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	api->DebugPushGroup("RmlUi Overlays");

	SetupContextRendering(api, renderTarget);

	// render context elements
	api->DebugPushGroup("Overlay Context");
	context.Render();
	api->DebugPopGroup();

	// blit results to target framebuffer 
	api->BindFramebuffer(targetFb);
	api->SetBlendFunction(render::E_BlendFactor::SourceAlpha, render::E_BlendFactor::OneMinusSourceAlpha);

	api->SetViewport(ivec2(0), render::Viewport::GetCurrentViewport()->GetDimensions());

	api->SetShader(m_RmlBlitShader.get());
	m_RmlBlitShader->Upload("uTexture"_hash, renderTarget.GetTexture());
	render::RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<render::primitives::Quad>();

	// reset pipeline state
	api->SetBlendEnabled(false);
	api->DebugPopGroup();
}

//----------------------------------
// GuiRenderer::RenderWorldContext
//
// Render a 3D world context
//
void GuiRenderer::RenderWorldContext(render::T_FbLoc const targetFb, 
	ContextRenderTarget& renderTarget, 
	Context& context, 
	mat4 const& transform,
	vec4 const& color,
	bool const enableDepth)
{
	if (!context.HasActiveDocuments())
	{
		return;
	}

	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	api->DebugPushGroup("RmlUi 3D Context");

	SetupContextRendering(api, renderTarget);

	api->DebugPushGroup("Context elements");
	context.Render();
	api->DebugPopGroup();

	// blit results to target framebuffer 
	api->BindFramebuffer(targetFb);
	api->SetBlendFunction(render::E_BlendFactor::SourceAlpha, render::E_BlendFactor::OneMinusSourceAlpha);

	api->SetDepthEnabled(enableDepth);
	api->SetViewport(ivec2(0), render::Viewport::GetCurrentViewport()->GetDimensions());

	api->SetShader(m_RmlBlit3DShader.get());
	m_RmlBlit3DShader->Upload("uTexture"_hash, renderTarget.GetTexture());
	m_RmlBlit3DShader->Upload("uTransform"_hash, transform);
	m_RmlBlit3DShader->Upload("uColor"_hash, color);
	render::RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<render::primitives::Quad>();

	// reset pipeline state
	api->SetBlendEnabled(false);
	api->SetDepthEnabled(false);

	api->DebugPopGroup();
}

//------------------------------------
// GuiRenderer::SetupContextRendering
//
// Prepare device for rendering UI contexts
//
void GuiRenderer::SetupContextRendering(render::I_GraphicsContextApi* const api, ContextRenderTarget &renderTarget)
{
	RmlGlobal::GetInstance()->SetGraphicsContext(ToPtr(api));

	// Set target framebuffer
	api->BindFramebuffer(renderTarget.GetFramebuffer());

	api->SetClearColor(vec4(0.f));
	api->Clear(render::E_ClearFlag::CF_Color);

	// view projection matrix
	ivec2 const iViewDim = renderTarget.GetDimensions();
	vec2 const viewDim = math::vecCast<float>(iViewDim);
	mat4 const viewProjection = math::orthographic(0.f, viewDim.x, viewDim.y, 0.f, -10000.f, 10000.f) * math::scale(vec3(1.f, -1.f, 1.f)); // v flip
	RmlGlobal::GetInstance()->SetRIView(iViewDim, viewProjection);

	// set shaders
	api->SetShader(m_RmlSdfShader.get());
	m_RmlSdfShader->Upload("uViewProjection"_hash, viewProjection);

	api->SetShader(m_GenericShader.get());
	m_GenericShader->Upload("uViewProjection"_hash, viewProjection);

	RmlGlobal::GetInstance()->SetRIShader(m_GenericShader, m_RmlSdfShader);

	// pipeline state
	api->SetViewport(ivec2(0), iViewDim);
	api->SetBlendEnabled(true);
	api->SetBlendEquation(render::E_BlendEquation::Add);
	api->SetBlendFunctionSeparate(render::E_BlendFactor::SourceAlpha, render::E_BlendFactor::One, // alpha channel always adds preventing gaps in the fb
		render::E_BlendFactor::OneMinusSourceAlpha, render::E_BlendFactor::One);
	api->SetCullEnabled(false);
	api->SetDepthEnabled(false);
}


} // namespace gui
} // namespace et

