#include "stdafx.h"
#include "ContextRenderTarget.h"

#include <EtRHI/GraphicsTypes/TextureData.h>


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
		m_Texture = Create<rhi::TextureData>(*other.m_Texture);
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

		rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

		device->GenFramebuffers(1, &m_Framebuffer);
		device->BindFramebuffer(m_Framebuffer);

		// target texture
		m_Texture = Create<rhi::TextureData>(rhi::E_ColorFormat::RGBA8, dim); // non float fb prevents alpha from exceeding 1
		m_Texture->AllocateStorage();
		m_Texture->SetParameters(rhi::TextureParameters(false));

		//Render Buffer for depth and stencil
		device->GenRenderBuffers(1, &m_Renderbuffer);
		device->BindRenderbuffer(m_Renderbuffer);
		device->SetRenderbufferStorage(rhi::E_RenderBufferFormat::Depth24_Stencil8, dim);

		// link it all together
		device->LinkRenderbufferToFbo(rhi::E_RenderBufferFormat::Depth24_Stencil8, m_Renderbuffer);
		device->LinkTextureToFbo2D(0, m_Texture->GetLocation(), 0);

		device->BindFramebuffer(0u);
	}
}

//----------------------------------------
// ContextRenderTarget::DeleteFramebuffer
//
void ContextRenderTarget::DeleteFramebuffer()
{
	if (m_Texture != nullptr)
	{
		rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

		device->DeleteRenderBuffers(1, &m_Renderbuffer);
		m_Texture = nullptr;
		device->DeleteFramebuffers(1, &m_Framebuffer);
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
