#include "stdafx.h"
#include "GuiRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/Util/PrimitiveRenderer.h>

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
	m_GenericShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("PostGenericUi.glsl"));
	m_RmlSdfShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("PostRmlUiSdf.glsl"));
	m_RmlBlitShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("PostRmlUiBlit.glsl"));
	m_RmlBlit3DShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/PostRmlUiBlit3D.glsl"));
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
void GuiRenderer::RenderContext(rhi::T_FbLoc const targetFb, ContextRenderTarget& renderTarget, Context& context, rhi::E_PolygonMode const polyMode)
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	device->DebugPushGroup("RmlUi Overlays");

	SetupContextRendering(device, renderTarget);

	device->SetPolygonMode(rhi::E_FaceCullMode::FrontBack, polyMode);

	// render context elements
	device->DebugPushGroup("Overlay Context");
	context.Render();
	device->DebugPopGroup();

	device->SetPolygonMode(rhi::E_FaceCullMode::FrontBack, rhi::E_PolygonMode::Fill);

	// blit results to target framebuffer 
	device->BindFramebuffer(targetFb);
	device->SetBlendFunction(rhi::E_BlendFactor::SourceAlpha, rhi::E_BlendFactor::OneMinusSourceAlpha);

	device->SetViewport(ivec2(0), rhi::Viewport::GetCurrentViewport()->GetDimensions());

	device->SetShader(m_RmlBlitShader.get());
	m_RmlBlitShader->Upload("uTexture"_hash, renderTarget.GetTexture());
	rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();

	// reset pipeline state
	device->SetBlendEnabled(false);
	device->DebugPopGroup();
}

//----------------------------------
// GuiRenderer::RenderWorldContext
//
// Render a 3D world context
//
void GuiRenderer::RenderWorldContext(rhi::T_FbLoc const targetFb, 
	ContextRenderTarget& renderTarget, 
	Context& context, 
	mat4 const& transform,
	vec4 const& color,
	bool const enableDepth,
	rhi::E_PolygonMode const polyMode)
{
	if (!context.HasActiveDocuments())
	{
		return;
	}

	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	device->DebugPushGroup("RmlUi 3D Context");

	device->SetPolygonMode(rhi::E_FaceCullMode::FrontBack, polyMode);

	SetupContextRendering(device, renderTarget);

	device->DebugPushGroup("Context elements");
	context.Render();
	device->DebugPopGroup();

	device->SetPolygonMode(rhi::E_FaceCullMode::FrontBack, rhi::E_PolygonMode::Fill);

	// blit results to target framebuffer 
	device->BindFramebuffer(targetFb);
	device->SetBlendFunction(rhi::E_BlendFactor::SourceAlpha, rhi::E_BlendFactor::OneMinusSourceAlpha);

	device->SetDepthEnabled(enableDepth);
	device->SetViewport(ivec2(0), rhi::Viewport::GetCurrentViewport()->GetDimensions());

	device->SetShader(m_RmlBlit3DShader.get());
	m_RmlBlit3DShader->Upload("uTexture"_hash, renderTarget.GetTexture());
	m_RmlBlit3DShader->Upload("uTransform"_hash, transform);
	m_RmlBlit3DShader->Upload("uColor"_hash, color);
	rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();

	// reset pipeline state
	device->SetBlendEnabled(false);
	device->SetDepthEnabled(false);

	device->DebugPopGroup();
}

//------------------------------------
// GuiRenderer::SetupContextRendering
//
// Prepare device for rendering UI contexts
//
void GuiRenderer::SetupContextRendering(rhi::I_RenderDevice* const device, ContextRenderTarget &renderTarget)
{
	RmlGlobal::GetInstance()->SetRenderDevice(ToPtr(device));

	// Set target framebuffer
	device->BindFramebuffer(renderTarget.GetFramebuffer());

	device->SetClearColor(vec4(0.f));
	device->Clear(rhi::E_ClearFlag::CF_Color);

	// view projection matrix
	ivec2 const iViewDim = renderTarget.GetDimensions();
	vec2 const viewDim = math::vecCast<float>(iViewDim);
	mat4 const viewProjection = math::orthographic(0.f, viewDim.x, viewDim.y, 0.f, -10000.f, 10000.f) * math::scale(vec3(1.f, -1.f, 1.f)); // v flip
	RmlGlobal::GetInstance()->SetRIView(iViewDim, viewProjection);

	// set shaders
	device->SetShader(m_RmlSdfShader.get());
	m_RmlSdfShader->Upload("uViewProjection"_hash, viewProjection);

	device->SetShader(m_GenericShader.get());
	m_GenericShader->Upload("uViewProjection"_hash, viewProjection);

	RmlGlobal::GetInstance()->SetRIShader(m_GenericShader, m_RmlSdfShader);

	// pipeline state
	device->SetViewport(ivec2(0), iViewDim);
	device->SetBlendEnabled(true);
	device->SetBlendEquation(rhi::E_BlendEquation::Add);
	device->SetBlendFunctionSeparate(rhi::E_BlendFactor::SourceAlpha, rhi::E_BlendFactor::One, // alpha channel always adds preventing gaps in the fb
		rhi::E_BlendFactor::OneMinusSourceAlpha, rhi::E_BlendFactor::One);
	device->SetCullEnabled(false);
	device->SetDepthEnabled(false);
}


} // namespace gui
} // namespace et

