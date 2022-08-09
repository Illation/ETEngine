#include "stdafx.h"
#include "FrameBuffer.h"

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/GraphicsTypes/TextureData.h>
#include <EtRHI/Util/PrimitiveRenderer.h>


namespace et {
namespace render {


FrameBuffer::FrameBuffer(std::string shaderFile, uint32 numTargets)
	: m_ShaderFile(shaderFile)
	, m_NumTargets(numTargets)
{ }

FrameBuffer::~FrameBuffer()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	if (m_VPCallbackId != rhi::T_ViewportEventDispatcher::INVALID_ID)
	{
		rhi::Viewport::GetCurrentViewport()->GetEventDispatcher().Unregister(m_VPCallbackId);
	}

	device->DeleteRenderBuffers(1, &m_RboDepthStencil);
	for (size_t i = 0; i < m_pTextureVec.size(); i++)
	{
		SafeDelete(m_pTextureVec[i]);
	}
	device->DeleteFramebuffers(1, &m_GlFrameBuffer);
}

void FrameBuffer::Initialize()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	//Load and compile Shaders
	m_pShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString(m_ShaderFile.c_str()));

	//GetAccessTo shader attributes
	device->SetShader(m_pShader.get());
	AccessShaderAttributes();

	//FrameBuffer
	device->GenFramebuffers(1, &m_GlFrameBuffer);

	GenerateFramebufferTextures();

	ET_ASSERT(device->IsFramebufferComplete(), "Creating framebuffer failed!");

	m_VPCallbackId = rhi::Viewport::GetCurrentViewport()->GetEventDispatcher().Register(rhi::E_ViewportEvent::VP_Resized, rhi::T_ViewportEventCallback(
		[this](rhi::T_ViewportEventFlags const, rhi::ViewportEventData const* const) -> void
		{
			ResizeFramebufferTextures();
		}));
}

void FrameBuffer::Enable(bool active)
{
	rhi::ContextHolder::GetRenderDevice()->BindFramebuffer(active ? m_GlFrameBuffer : 0);
}

void FrameBuffer::Draw()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	device->SetDepthEnabled(false);
	device->SetShader(m_pShader.get());

	UploadDerivedVariables();

	rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
}


void FrameBuffer::GenerateFramebufferTextures()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();
	ivec2 const dim = rhi::Viewport::GetCurrentViewport()->GetDimensions();

	device->BindFramebuffer(m_GlFrameBuffer);

	//Textures
	m_pTextureVec.reserve(m_NumTargets);

	rhi::TextureParameters params(false);
	params.wrapS = rhi::E_TextureWrapMode::ClampToEdge;
	params.wrapT = rhi::E_TextureWrapMode::ClampToEdge;
	//Depth buffer
	if (m_CaptureDepth)
	{
		rhi::TextureData* depthMap = new rhi::TextureData(rhi::E_ColorFormat::Depth24, dim);
		depthMap->AllocateStorage();
		device->LinkTextureToFboDepth(depthMap->GetLocation());
		depthMap->SetParameters(params);
		depthMap->CreateHandle();
		m_pTextureVec.emplace_back(depthMap);
	}

	//Color buffers
	for (uint32 i = 0; i < m_NumTargets; i++)
	{
		rhi::TextureData* colorBuffer = new rhi::TextureData(rhi::E_ColorFormat::RGBA16f, dim);
		colorBuffer->AllocateStorage();
		device->LinkTextureToFbo2D(i, colorBuffer->GetLocation(), 0);
		colorBuffer->SetParameters(params, true);
		colorBuffer->CreateHandle();
		m_pTextureVec.emplace_back(colorBuffer);
	}

	//Render Buffer for depth and stencil
	if (!m_CaptureDepth)
	{
		device->GenRenderBuffers(1, &m_RboDepthStencil);
		device->BindRenderbuffer(m_RboDepthStencil);
		device->SetRenderbufferStorage(rhi::E_RenderBufferFormat::Depth24_Stencil8, dim);
		device->LinkRenderbufferToFbo(rhi::E_RenderBufferFormat::Depth24_Stencil8, m_RboDepthStencil);
	}

	device->SetDrawBufferCount(static_cast<size_t>(m_NumTargets));
}

void FrameBuffer::ResizeFramebufferTextures()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();
	ivec2 const dim = rhi::Viewport::GetCurrentViewport()->GetDimensions();

	ET_ASSERT(m_pTextureVec.size() > 0);

	if(dim.x > m_pTextureVec[0]->GetResolution().x || dim.x > m_pTextureVec[0]->GetResolution().y)
	{
		device->DeleteRenderBuffers( 1, &m_RboDepthStencil );
		for(uint32 i = 0; i < m_pTextureVec.size(); i++)
		{
			SafeDelete( m_pTextureVec[i] );
		}
		m_pTextureVec.clear();
		device->DeleteFramebuffers( 1, &m_GlFrameBuffer );
		device->GenFramebuffers( 1, &m_GlFrameBuffer );
		GenerateFramebufferTextures();
		return;
	}

	uint32 offset = 0;
	if(m_CaptureDepth)
	{
		m_pTextureVec[0]->Resize(dim);
		++offset;
	}
	else 
	{
		//completely regenerate the renderbuffer object
		device->BindFramebuffer(m_GlFrameBuffer);
		device->DeleteRenderBuffers( 1, &m_RboDepthStencil );
		device->GenRenderBuffers(1, &m_RboDepthStencil);
		device->BindRenderbuffer(m_RboDepthStencil);
		device->SetRenderbufferStorage(rhi::E_RenderBufferFormat::Depth24_Stencil8, dim);
		device->LinkRenderbufferToFbo(rhi::E_RenderBufferFormat::Depth24_Stencil8, m_RboDepthStencil);
	}
	assert( m_pTextureVec.size() >= offset + m_NumTargets );
	for(uint32 i = offset; i < offset + m_NumTargets; ++i)
	{
		m_pTextureVec[i]->Resize(dim);
	}
}


} // namespace render
} // namespace et
