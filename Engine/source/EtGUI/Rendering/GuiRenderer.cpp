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
	m_RmlShader = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("Shaders/PostRmlUi.glsl"));
	m_RmlSdfShader = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("Shaders/PostRmlUiSdf.glsl"));
	m_RmlBlitShader = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("Shaders/PostRmlUiBlit.glsl"));
}

//----------------------------------
// GuiRenderer::Deinit
//
void GuiRenderer::Deinit()
{
	DeleteFramebuffer();

	m_RmlGlobal = nullptr;

	m_RmlShader = nullptr;
	m_RmlBlitShader = nullptr;
	m_RmlSdfShader = nullptr;
}

//----------------------------------
// GuiRenderer::RenderContexts
//
// Render a list of contexts to the target framebuffer
//
void GuiRenderer::RenderContexts(render::Viewport const* const viewport, render::T_FbLoc const targetFb, Context* const contexts, size_t const count)
{
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	RmlGlobal::GetInstance()->SetGraphicsContext(ToPtr(api));

	// Set target framebuffer
	ivec2 const iViewDim = viewport->GetDimensions();
	if ((m_RmlTex == nullptr) || !math::nearEqualsV(m_RmlTex->GetResolution(), iViewDim))
	{
		GenerateFramebuffer(iViewDim);
	}

	api->BindFramebuffer(m_RmlFb);

	api->SetClearColor(vec4(0.f));
	api->Clear(render::E_ClearFlag::CF_Color);

	// view projection matrix
	vec2 const viewDim = math::vecCast<float>(iViewDim);
	mat4 const viewProjection = math::orthographic(0.f, viewDim.x, viewDim.y, 0.f, -10000.f, 10000.f) * math::scale(vec3(1.f, -1.f, 1.f)); // v flip
	RmlGlobal::GetInstance()->SetRIView(iViewDim, viewProjection);

	// set shaders
	api->SetShader(m_RmlSdfShader.get());
	m_RmlSdfShader->Upload("uViewProjection"_hash, viewProjection);

	api->SetShader(m_RmlShader.get());
	m_RmlShader->Upload("uViewProjection"_hash, viewProjection);

	RmlGlobal::GetInstance()->SetRIShader(m_RmlShader, m_RmlSdfShader);

	// pipeline state
	api->SetBlendEnabled(true);
	api->SetBlendEquation(render::E_BlendEquation::Add);
	api->SetBlendFunctionSeparate(render::E_BlendFactor::SourceAlpha, render::E_BlendFactor::One, // alpha channel always adds preventing gaps in the fb
		render::E_BlendFactor::OneMinusSourceAlpha, render::E_BlendFactor::One);
	api->SetCullEnabled(false);
	api->SetDepthEnabled(false);

	// render each context
	for (size_t cIdx = 0u; cIdx < count; ++cIdx)
	{
		Context& context = contexts[cIdx];
		if (context.IsActive() && context.IsDocumentLoaded())
		{
			context.Render();
		}
	}

	// blit results to target framebuffer - this can also be used in the future to transform the UI into viewspace
	api->BindFramebuffer(targetFb);
	api->SetBlendFunction(render::E_BlendFactor::SourceAlpha, render::E_BlendFactor::OneMinusSourceAlpha);

	api->SetShader(m_RmlBlitShader.get());
	m_RmlBlitShader->Upload("uTexture"_hash, static_cast<render::TextureData const*>(m_RmlTex.Get()));
	render::RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<render::primitives::Quad>();

	// reset pipeline state
	api->SetBlendEnabled(false);
}

//----------------------------------
// GuiRenderer::GenerateFramebuffer
//
void GuiRenderer::GenerateFramebuffer(ivec2 const dim)
{
	DeleteFramebuffer();

	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	api->GenFramebuffers(1, &m_RmlFb);
	api->BindFramebuffer(m_RmlFb);

	// target texture
	m_RmlTex = Create<render::TextureData>(render::E_ColorFormat::RGBA8, dim); // non float fb prevents alpha from exceeding 1
	m_RmlTex->AllocateStorage();
	m_RmlTex->SetParameters(render::TextureParameters(false));

	//Render Buffer for depth and stencil
	api->GenRenderBuffers(1, &m_RmlRbo);
	api->BindRenderbuffer(m_RmlRbo);
	api->SetRenderbufferStorage(render::E_RenderBufferFormat::Depth24_Stencil8, dim);

	// link it all together
	api->LinkRenderbufferToFbo(render::E_RenderBufferFormat::Depth24_Stencil8, m_RmlRbo);
	api->LinkTextureToFbo2D(0, m_RmlTex->GetLocation(), 0);

	api->BindFramebuffer(0u);
}

//--------------------------------
// GuiRenderer::DeleteFramebuffer
//
void GuiRenderer::DeleteFramebuffer()
{
	if (m_RmlTex != nullptr)
	{
		render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

		api->DeleteRenderBuffers(1, &m_RmlRbo);
		m_RmlTex = nullptr;
		api->DeleteFramebuffers(1, &m_RmlFb);
	}
}


} // namespace gui
} // namespace et

