#include "stdafx.h"
#include "SpriteRenderer.h"

#include <glad/glad.h>

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>


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
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteVertexArrays(1, &m_VAO);
	api->DeleteBuffers(1, &m_VBO);

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
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	m_Shader = ResourceManager::Instance()->GetAssetData<ShaderData>("PostSprite.glsl"_hash);

	api->SetShader(m_Shader.get());

	m_Shader->Upload("uTexture"_hash, 0);
	m_Shader->Upload("u3DTexture"_hash, 1);

	//Generate buffers and arrays
	api->GenerateVertexArrays(1, &m_VAO);
	api->GenerateBuffers(1, &m_VBO);

	//bind
	api->BindVertexArray(m_VAO);
	api->BindBuffer(GL_ARRAY_BUFFER, m_VBO);

	//set data and attributes
	api->SetBufferData(GL_ARRAY_BUFFER, m_BufferSize, NULL, GL_DYNAMIC_DRAW);

	//input layout
	api->SetVertexAttributeArrayEnabled(0, true);
	api->SetVertexAttributeArrayEnabled(1, true);
	api->SetVertexAttributeArrayEnabled(2, true);
	api->SetVertexAttributeArrayEnabled(3, true);

	int32 const vertSize = sizeof(SpriteVertex);
	api->DefineVertexAttribIPointer(0, (GLint)1, GL_UNSIGNED_INT, vertSize, (GLvoid*)offsetof(SpriteVertex, TextureId));
	api->DefineVertexAttributePointer(1, (GLint)4, GL_FLOAT, GL_FALSE, vertSize, (GLvoid*)offsetof(SpriteVertex, TransformData));
	api->DefineVertexAttributePointer(2, (GLint)4, GL_FLOAT, GL_FALSE, vertSize, (GLvoid*)offsetof(SpriteVertex, TransformData2));
	api->DefineVertexAttributePointer(3, (GLint)4, GL_FLOAT, GL_FALSE, vertSize, (GLvoid*)offsetof(SpriteVertex, Color));

	//unbind
	api->BindBuffer(GL_ARRAY_BUFFER, 0);
	api->BindVertexArray(0);

	CalculateTransform();

	//Create empty dummy texture
	m_EmptyTex = new TextureData(1, 1, GL_RGB, GL_RGB, GL_FLOAT);

	m_EmptyTex->Build((void*)(vec4(1).data.data()));

	TextureParameters params;
	m_EmptyTex->SetParameters(params);

	Config::GetInstance()->GetWindow().WindowResizeEvent.AddListener(std::bind(&SpriteRenderer::OnWindowResize, this));
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
		finalScale = scale * etm::vecCast<float>(viewSize);
	}
	break;

	case E_ScalingMode::Texture:
		finalScale = scale * etm::vecCast<float>(tex->GetResolution()) / Config::GetInstance()->GetGraphics().TextureScaleFactor;
		break;

	case E_ScalingMode::TextureAbs:
		finalScale = scale * etm::vecCast<float>(tex->GetResolution());
		break;

	default:
		ET_ASSERT(true, "Unhandled Sprite Scaling Mode!");
		break;
	}

	vertex.TransformData2 = vec4(pivot, finalScale);
	vertex.Color = color;

	if (tex->GetTarget() == GL_TEXTURE_3D)
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

	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	UpdateBuffer();

	api->BindVertexArray(m_VAO);

	CalculateTransform();
	api->SetShader(m_Shader.get());
	api->SetActiveTexture(0);
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
		if (texData->GetTarget() == GL_TEXTURE_2D)
		{
			api->SetActiveTexture(0);
			m_Shader->Upload("uDraw3D"_hash, false);
		}
		else
		{
			api->SetActiveTexture(1);
			m_Shader->Upload("uDraw3D"_hash, true);
			m_Shader->Upload("uLayer"_hash, m_Layer);
		}
		api->BindTexture(texData->GetTarget(), texData->GetHandle());

		//Draw
		api->DrawArrays(GL_POINTS, batchOffset, batchSize);

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
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//Bind Object vertex array
	api->BindVertexArray(m_VAO);

	//Send the vertex buffer again
	api->BindBuffer(GL_ARRAY_BUFFER, m_VBO);

	bool bufferResize = m_Sprites.size() * sizeof( SpriteVertex ) > m_BufferSize;
	if(!m_VBO || bufferResize) //first creation or resize
	{
		if (bufferResize)
		{
			m_BufferSize = (uint32)m_Sprites.size() * sizeof( SpriteVertex );
		}

		api->SetBufferData(GL_ARRAY_BUFFER, m_BufferSize, m_Sprites.data(), GL_DYNAMIC_DRAW);
	}
	else
	{
		GLvoid* p = api->MapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
		memcpy( p, m_Sprites.data(), m_Sprites.size() * sizeof( SpriteVertex ) );
		api->UnmapBuffer( GL_ARRAY_BUFFER );
	}


	api->BindBuffer(GL_ARRAY_BUFFER, 0);

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
