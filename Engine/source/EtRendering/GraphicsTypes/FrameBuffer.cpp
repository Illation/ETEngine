#include "stdafx.h"
#include "FrameBuffer.h"

#include "Shader.h"
#include "TextureData.h"

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


FrameBuffer::FrameBuffer(std::string shaderFile, E_DataType const format, uint32 numTargets)
	: m_ShaderFile(shaderFile)
	, m_Format(format)
	, m_NumTargets(numTargets)
{ }

FrameBuffer::~FrameBuffer()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteRenderBuffers(1, &m_RboDepthStencil);
	for (size_t i = 0; i < m_pTextureVec.size(); i++)
	{
		SafeDelete(m_pTextureVec[i]);
	}
	api->DeleteFramebuffers(1, &m_GlFrameBuffer);
}

void FrameBuffer::Initialize()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//Load and compile Shaders
	m_pShader = ResourceManager::Instance()->GetAssetData<ShaderData>(GetHash(FileUtil::ExtractName(m_ShaderFile)));

	//GetAccessTo shader attributes
	api->SetShader(m_pShader.get());
	AccessShaderAttributes();

	//FrameBuffer
	api->GenFramebuffers(1, &m_GlFrameBuffer);

	GenerateFramebufferTextures();

	ET_ASSERT(api->IsFramebufferComplete(), "Creating framebuffer failed!");

	Viewport::GetCurrentViewport()->GetResizeEvent().AddListener(std::bind( &FrameBuffer::ResizeFramebufferTextures, this));
}

void FrameBuffer::AccessShaderAttributes()
{
	m_pShader->Upload("texFramebuffer"_hash, 0);
}

void FrameBuffer::Enable(bool active)
{
	Viewport::GetCurrentApiContext()->BindFramebuffer(active ? m_GlFrameBuffer : 0);
}

void FrameBuffer::Draw()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->SetDepthEnabled(false);
	api->SetShader(m_pShader.get());

	UploadDerivedVariables();

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
}


void FrameBuffer::GenerateFramebufferTextures()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();
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
		TextureData* depthMap = new TextureData(dim, E_ColorFormat::Depth24, E_ColorFormat::Depth, E_DataType::Float);
		depthMap->Build();
		api->LinkTextureToFboDepth(depthMap->GetLocation());
		depthMap->SetParameters(params);
		depthMap->CreateHandle();
		m_pTextureVec.emplace_back(depthMap);
	}

	//Color buffers
	for (uint32 i = 0; i < m_NumTargets; i++)
	{
		TextureData* colorBuffer = new TextureData(dim, E_ColorFormat::RGBA16f, E_ColorFormat::RGBA, m_Format);
		colorBuffer->Build();
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
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();
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
