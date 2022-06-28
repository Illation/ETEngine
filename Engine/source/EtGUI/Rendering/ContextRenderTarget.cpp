#include "stdafx.h"
#include "ContextRenderTarget.h"

#include <EtRendering/GraphicsTypes/TextureData.h>


namespace et {
namespace gui {


//=======================
// Context Render Target 
//=======================


//----------------------------------------
// ContextRenderTarget::copy c-tor
//
ContextRenderTarget::ContextRenderTarget(ContextRenderTarget const& other)
{
	*this = other;
}

//----------------------------------------
// ContextRenderTarget::copy operator
//
ContextRenderTarget& ContextRenderTarget::operator=(ContextRenderTarget const& other)
{
	m_Framebuffer = other.m_Framebuffer;
	m_Renderbuffer = other.m_Renderbuffer;
	if (other.m_Texture != nullptr)
	{
		m_Texture = Create<render::TextureData>(*other.m_Texture);
	}

	return *this;
}

//----------------------------
// ContextRenderTarget::d-tor
//
ContextRenderTarget::~ContextRenderTarget()
{
	DeleteFramebuffer();
}

//------------------------------------------
// ContextRenderTarget::UpdateForDimensions
//
void ContextRenderTarget::UpdateForDimensions(ivec2 const dim)
{
	if ((m_Texture == nullptr) || !math::nearEqualsV(m_Texture->GetResolution(), dim))
	{
		DeleteFramebuffer();

		render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

		api->GenFramebuffers(1, &m_Framebuffer);
		api->BindFramebuffer(m_Framebuffer);

		// target texture
		m_Texture = Create<render::TextureData>(render::E_ColorFormat::RGBA8, dim); // non float fb prevents alpha from exceeding 1
		m_Texture->AllocateStorage();
		m_Texture->SetParameters(render::TextureParameters(false));

		//Render Buffer for depth and stencil
		api->GenRenderBuffers(1, &m_Renderbuffer);
		api->BindRenderbuffer(m_Renderbuffer);
		api->SetRenderbufferStorage(render::E_RenderBufferFormat::Depth24_Stencil8, dim);

		// link it all together
		api->LinkRenderbufferToFbo(render::E_RenderBufferFormat::Depth24_Stencil8, m_Renderbuffer);
		api->LinkTextureToFbo2D(0, m_Texture->GetLocation(), 0);

		api->BindFramebuffer(0u);
	}
}

//----------------------------------------
// ContextRenderTarget::DeleteFramebuffer
//
void ContextRenderTarget::DeleteFramebuffer()
{
	if (m_Texture != nullptr)
	{
		render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

		api->DeleteRenderBuffers(1, &m_Renderbuffer);
		m_Texture = nullptr;
		api->DeleteFramebuffers(1, &m_Framebuffer);
	}
}

//----------------------------------------
// ContextRenderTarget::GetDimensions
//
ivec2 ContextRenderTarget::GetDimensions() const
{
	if (m_Texture == nullptr)
	{
		return ivec2();
	}

	return m_Texture->GetResolution();
}


} // namespace gui
} // namespace et
