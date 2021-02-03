#include "stdafx.h"
#include "FrameBuffer.h"

#include "Shader.h"
#include "TextureData.h"

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace render {


FrameBuffer::FrameBuffer(std::string shaderFile, uint32 numTargets)
	: m_ShaderFile(shaderFile)
	, m_NumTargets(numTargets)
{ }

FrameBuffer::~FrameBuffer()
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	if (m_VPCallbackId != render::T_ViewportEventDispatcher::INVALID_ID)
	{
		Viewport::GetCurrentViewport()->GetEventDispatcher().Unregister(m_VPCallbackId);
	}

	api->DeleteRenderBuffers(1, &m_RboDepthStencil);
	for (size_t i = 0; i < m_pTextureVec.size(); i++)
	{
		SafeDelete(m_pTextureVec[i]);
	}
	api->DeleteFramebuffers(1, &m_GlFrameBuffer);
}

void FrameBuffer::Initialize()
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	//Load and compile Shaders
	m_pShader = core::ResourceManager::Instance()->GetAssetData<ShaderData>(core::HashString(m_ShaderFile.c_str()));

	//GetAccessTo shader attributes
	api->SetShader(m_pShader.get());
	AccessShaderAttributes();

	//FrameBuffer
	api->GenFramebuffers(1, &m_GlFrameBuffer);

	GenerateFramebufferTextures();

	ET_ASSERT(api->IsFramebufferComplete(), "Creating framebuffer failed!");

	m_VPCallbackId = Viewport::GetCurrentViewport()->GetEventDispatcher().Register(render::E_ViewportEvent::VP_Resized, render::T_ViewportEventCallback(
		[this](render::T_ViewportEventFlags const, render::ViewportEventData const* const) -> void
		{
			ResizeFramebufferTextures();
		}));
}

void FrameBuffer::Enable(bool active)
{
	ContextHolder::GetRenderContext()->BindFramebuffer(active ? m_GlFrameBuffer : 0);
}

void FrameBuffer::Draw()
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	api->SetDepthEnabled(false);
	api->SetShader(m_pShader.get());

	UploadDerivedVariables();

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
}


void FrameBuffer::GenerateFramebufferTextures()
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();
	ivec2 const dim = Viewport::GetCurrentViewport()->GetDimensions();

	api->BindFramebuffer(m_GlFrameBuffer);

	//Textures
	m_pTextureVec.reserve(m_NumTargets);

	TextureParameters params(false);
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;
	//Depth buffer
	if (m_CaptureDepth)
	{
		TextureData* depthMap = new TextureData(E_ColorFormat::Depth24, dim);
		depthMap->AllocateStorage();
		api->LinkTextureToFboDepth(depthMap->GetLocation());
		depthMap->SetParameters(params);
		depthMap->CreateHandle();
		m_pTextureVec.emplace_back(depthMap);
	}

	//Color buffers
	for (uint32 i = 0; i < m_NumTargets; i++)
	{
		TextureData* colorBuffer = new TextureData(E_ColorFormat::RGBA16f, dim);
		colorBuffer->AllocateStorage();
		api->LinkTextureToFbo2D(i, colorBuffer->GetLocation(), 0);
		colorBuffer->SetParameters(params, true);
		colorBuffer->CreateHandle();
		m_pTextureVec.emplace_back(colorBuffer);
	}

	//Render Buffer for depth and stencil
	if (!m_CaptureDepth)
	{
		api->GenRenderBuffers(1, &m_RboDepthStencil);
		api->BindRenderbuffer(m_RboDepthStencil);
		api->SetRenderbufferStorage(E_RenderBufferFormat::Depth24_Stencil8, dim);
		api->LinkRenderbufferToFbo(E_RenderBufferFormat::Depth24_Stencil8, m_RboDepthStencil);
	}

	api->SetDrawBufferCount(static_cast<size_t>(m_NumTargets));
}

void FrameBuffer::ResizeFramebufferTextures()
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();
	ivec2 const dim = Viewport::GetCurrentViewport()->GetDimensions();

	ET_ASSERT(m_pTextureVec.size() > 0);

	if(dim.x > m_pTextureVec[0]->GetResolution().x || dim.x > m_pTextureVec[0]->GetResolution().y)
	{
		api->DeleteRenderBuffers( 1, &m_RboDepthStencil );
		for(uint32 i = 0; i < m_pTextureVec.size(); i++)
		{
			SafeDelete( m_pTextureVec[i] );
		}
		m_pTextureVec.clear();
		api->DeleteFramebuffers( 1, &m_GlFrameBuffer );
		api->GenFramebuffers( 1, &m_GlFrameBuffer );
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
		api->BindFramebuffer(m_GlFrameBuffer);
		api->DeleteRenderBuffers( 1, &m_RboDepthStencil );
		api->GenRenderBuffers(1, &m_RboDepthStencil);
		api->BindRenderbuffer(m_RboDepthStencil);
		api->SetRenderbufferStorage(E_RenderBufferFormat::Depth24_Stencil8, dim);
		api->LinkRenderbufferToFbo(E_RenderBufferFormat::Depth24_Stencil8, m_RboDepthStencil);
	}
	assert( m_pTextureVec.size() >= offset + m_NumTargets );
	for(uint32 i = offset; i < offset + m_NumTargets; ++i)
	{
		m_pTextureVec[i]->Resize(dim);
	}
}


} // namespace render
} // namespace et
