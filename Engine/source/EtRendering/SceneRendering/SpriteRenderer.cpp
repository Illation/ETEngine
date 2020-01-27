#include "stdafx.h"
#include "SpriteRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace render {


//====================
// Sprite Renderer
//====================


// construct destruct
//--------------------

//---------------------------------
// SpriteRenderer::d-tor
//
// Clean up Sprite renderer
//
SpriteRenderer::~SpriteRenderer()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	if (m_VPCallbackId != render::T_ViewportEventDispatcher::INVALID_ID)
	{
		Viewport::GetCurrentViewport()->GetEventDispatcher().Unregister(m_VPCallbackId);
	}

	api->DeleteVertexArray(m_VAO);
	api->DeleteBuffer(m_VBO);

	m_Sprites.clear();
	m_Textures.clear();

	delete m_EmptyTex; 
	m_EmptyTex = nullptr;
}

//---------------------------------
// SpriteRenderer::Initialize
//
void SpriteRenderer::Initialize()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	m_Shader = core::ResourceManager::Instance()->GetAssetData<ShaderData>(core::HashString("PostSprite.glsl"));

	//Generate buffers and arrays
	m_VAO = api->CreateVertexArray();
	m_VBO = api->CreateBuffer();

	//bind
	api->BindVertexArray(m_VAO);
	api->BindBuffer(E_BufferType::Vertex, m_VBO);

	//set data and attributes
	api->SetBufferData(E_BufferType::Vertex, m_BufferSize, nullptr, E_UsageHint::Dynamic);

	//input layout
	api->SetVertexAttributeArrayEnabled(0, true);
	api->SetVertexAttributeArrayEnabled(1, true);
	api->SetVertexAttributeArrayEnabled(2, true);
	api->SetVertexAttributeArrayEnabled(3, true);

	int32 const vertSize = sizeof(SpriteVertex);
	api->DefineVertexAttribIPointer(0, 1, E_DataType::UInt, vertSize, offsetof(SpriteVertex, TextureId));
	api->DefineVertexAttributePointer(1, 4, E_DataType::Float, false, vertSize, offsetof(SpriteVertex, TransformData));
	api->DefineVertexAttributePointer(2, 4, E_DataType::Float, false, vertSize, offsetof(SpriteVertex, TransformData2));
	api->DefineVertexAttributePointer(3, 4, E_DataType::Float, false, vertSize, offsetof(SpriteVertex, Color));

	//unbind
	api->BindBuffer(E_BufferType::Vertex, 0);
	api->BindVertexArray(0);

	CalculateTransform();

	//Create empty dummy texture
	m_EmptyTex = new TextureData(ivec2(1), E_ColorFormat::RGB, E_ColorFormat::RGB, E_DataType::Float);

	m_EmptyTex->Build((void*)(vec4(1).data.data()));

	TextureParameters params;
	m_EmptyTex->SetParameters(params);

	m_VPCallbackId = Viewport::GetCurrentViewport()->GetEventDispatcher().Register(render::E_ViewportEvent::VP_Resized, render::T_ViewportEventCallback(
		[this](render::T_ViewportEventFlags const, render::ViewportEventData const* const) -> void
		{
			OnWindowResize();
		}));
}


// Functionality
//---------------

//---------------------------------
// SpriteRenderer::Draw
//
// Places a new Sprite in the Draw queue for later rendering, sets up its vertex data
//
void SpriteRenderer::Draw(TextureData const* tex,
	vec2 const& position,
	vec4 const& color,
	vec2 const& pivot,
	vec2 const& scale,
	float const rotation,
	float const depth,
	E_ScalingMode const mode,
	float const layer)
{
	SpriteVertex vertex;

	if (tex == nullptr)
	{
		tex = m_EmptyTex;
	}

	auto const foundTexIt = find(m_Textures.cbegin(), m_Textures.cend(), tex);
	if (foundTexIt == m_Textures.cend())
	{
		m_Textures.emplace_back(tex);
		vertex.TextureId = static_cast<uint32>(m_Textures.size() - 1);
	}
	else
	{
		vertex.TextureId = static_cast<uint32>(foundTexIt - m_Textures.cbegin());
	}

	vertex.TransformData = vec4(position, depth, rotation);

	vec2 finalScale;
	switch (mode)
	{
	case E_ScalingMode::Pixel:
		finalScale = scale;
		//no modification required
		break;

	case E_ScalingMode::Screen:
	{
		ivec2 viewPos, viewSize;
		Viewport::GetCurrentApiContext()->GetViewport(viewPos, viewSize);
		finalScale = scale * math::vecCast<float>(viewSize);
	}
	break;

	case E_ScalingMode::Texture:
		finalScale = scale * math::vecCast<float>(tex->GetResolution()) / RenderingSystems::Instance()->GetGraphicsSettings().TextureScaleFactor;
		break;

	case E_ScalingMode::TextureAbs:
		finalScale = scale * math::vecCast<float>(tex->GetResolution());
		break;

	default:
		ET_ASSERT(true, "Unhandled Sprite Scaling Mode!");
		break;
	}

	vertex.TransformData2 = vec4(pivot, finalScale);
	vertex.Color = color;

	if (tex->GetTargetType() == E_TextureType::Texture3D)
	{
		m_Layer = layer;
	}

	m_Sprites.push_back(vertex);
}

//---------------------------------
// SpriteRenderer::OnWindowResize
//
// Recalculates transform basis to preserve sprite scaling
//
void SpriteRenderer::OnWindowResize()
{
	CalculateTransform();
}

//---------------------------------
// SpriteRenderer::Draw
//
// Sends all cached sprite vertices to the GPU
//
void SpriteRenderer::Draw()
{
	if (m_Sprites.size() <= 0)
	{
		return;
	}

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	UpdateBuffer();

	api->BindVertexArray(m_VAO);

	CalculateTransform();
	api->SetShader(m_Shader.get());

	m_Shader->Upload("uTransform"_hash, m_Transform);

	uint32 batchSize = 1;
	uint32 batchOffset = 0;
	uint32 spriteCount = (uint32)m_Sprites.size();
	for (uint32 i = 0; i < spriteCount; ++i)
	{
		if (i < (spriteCount - 1) && m_Sprites[i].TextureId == m_Sprites[i + 1].TextureId)
		{
			++batchSize;
			continue;
		}

		TextureData const* const texData = m_Textures[m_Sprites[i].TextureId];
		if (texData->GetTargetType() == E_TextureType::Texture2D)
		{
			m_Shader->Upload("uDraw3D"_hash, false);
			m_Shader->Upload("uTexture"_hash, texData);
		}
		else
		{
			m_Shader->Upload("uDraw3D"_hash, true);
			m_Shader->Upload("u3DTexture"_hash, texData);
			m_Shader->Upload("uLayer"_hash, m_Layer);
		}

		//Draw
		api->DrawArrays(E_DrawMode::Points, batchOffset, batchSize);

		batchOffset += batchSize;
		batchSize = 1;
	}

	api->BindVertexArray(0);

	m_Sprites.clear();
	m_Textures.clear();
}


// Utility
//---------

//---------------------------------
// SpriteRenderer::UpdateBuffer
//
// Recalculates the Vertex buffer
//
void SpriteRenderer::UpdateBuffer()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//Bind Object vertex array
	api->BindVertexArray(m_VAO);

	//Send the vertex buffer again
	api->BindBuffer(E_BufferType::Vertex, m_VBO);

	bool bufferResize = m_Sprites.size() * sizeof( SpriteVertex ) > m_BufferSize;
	if(!m_VBO || bufferResize) //first creation or resize
	{
		if (bufferResize)
		{
			m_BufferSize = (uint32)m_Sprites.size() * sizeof( SpriteVertex );
		}

		api->SetBufferData(E_BufferType::Vertex, m_BufferSize, m_Sprites.data(), E_UsageHint::Dynamic);
	}
	else
	{
		void* p = api->MapBuffer(E_BufferType::Vertex, E_AccessMode::Write);
		memcpy( p, m_Sprites.data(), m_Sprites.size() * sizeof( SpriteVertex ) );
		api->UnmapBuffer(E_BufferType::Vertex);
	}


	api->BindBuffer(E_BufferType::Vertex, 0);

	//Done Modifying
	api->BindVertexArray(0);
}

//---------------------------------
// SpriteRenderer::CalculateTransform
//
// Recalculates the transformation matrix used by the shader
//
void SpriteRenderer::CalculateTransform()
{
	ivec2 viewPos, viewSize;
	Viewport::GetCurrentApiContext()->GetViewport(viewPos, viewSize);

	int32 const width = viewSize.x;
	int32 const height = viewSize.y;

	float const scaleX = (width > 0) ? 2.f / width : 0;
	float const scaleY = (height > 0) ? 2.f / height : 0;

	m_Transform = mat4({
		scaleX,	0,			0,		0,
		0,		-scaleY,	0,		0,
		0,		0,			1,		0,
		-1,		1,			0,		1 });
}


} // namespace render
} // namespace et
