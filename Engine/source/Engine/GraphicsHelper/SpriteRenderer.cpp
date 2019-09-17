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
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);

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
	m_Shader = ResourceManager::Instance()->GetAssetData<ShaderData>("PostSprite.glsl"_hash);

	STATE->SetShader(m_Shader.get());

	m_Shader->Upload("uTexture"_hash, 0);
	m_Shader->Upload("u3DTexture"_hash, 1);

	//Generate buffers and arrays
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	//bind
	STATE->BindVertexArray(m_VAO);
	STATE->BindBuffer(GL_ARRAY_BUFFER, m_VBO);

	//set data and attributes
	glBufferData(GL_ARRAY_BUFFER, m_BufferSize, NULL, GL_DYNAMIC_DRAW);

	//input layout
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glVertexAttribIPointer(0, (GLint)1, GL_UNSIGNED_INT, (GLsizei)sizeof(SpriteVertex), (GLvoid*)offsetof(SpriteVertex, TextureId));
	glVertexAttribPointer(1, (GLint)4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(SpriteVertex), (GLvoid*)offsetof(SpriteVertex, TransformData));
	glVertexAttribPointer(2, (GLint)4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(SpriteVertex), (GLvoid*)offsetof(SpriteVertex, TransformData2));
	glVertexAttribPointer(3, (GLint)4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(SpriteVertex), (GLvoid*)offsetof(SpriteVertex, Color));

	//unbind
	STATE->BindBuffer(GL_ARRAY_BUFFER, 0);
	STATE->BindVertexArray(0);

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
		STATE->GetViewport(viewPos, viewSize);
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
		return;

	UpdateBuffer();

	STATE->BindVertexArray(m_VAO);

	CalculateTransform();
	STATE->SetShader(m_Shader.get());
	STATE->SetActiveTexture(0);
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
			STATE->SetActiveTexture(0);
			m_Shader->Upload("uDraw3D"_hash, false);
		}
		else
		{
			STATE->SetActiveTexture(1);
			m_Shader->Upload("uDraw3D"_hash, true);
			m_Shader->Upload("uLayer"_hash, m_Layer);
		}
		STATE->BindTexture(texData->GetTarget(), texData->GetHandle());

		//Draw
		STATE->DrawArrays(GL_POINTS, batchOffset, batchSize);

		batchOffset += batchSize;
		batchSize = 1;
	}

	STATE->BindVertexArray(0);

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
	//Bind Object vertex array
	STATE->BindVertexArray(m_VAO);

	//Send the vertex buffer again
	STATE->BindBuffer(GL_ARRAY_BUFFER, m_VBO);

	bool bufferResize = m_Sprites.size() * sizeof( SpriteVertex ) > m_BufferSize;
	if(!m_VBO || bufferResize) //first creation or resize
	{
		if (bufferResize)
		{
			m_BufferSize = (uint32)m_Sprites.size() * sizeof( SpriteVertex );
		}

		glBufferData(GL_ARRAY_BUFFER, m_BufferSize, m_Sprites.data(), GL_DYNAMIC_DRAW);
	}
	else
	{
		GLvoid* p = glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
		memcpy( p, m_Sprites.data(), m_Sprites.size() * sizeof( SpriteVertex ) );
		glUnmapBuffer( GL_ARRAY_BUFFER );
	}


	STATE->BindBuffer(GL_ARRAY_BUFFER, 0);

	//Done Modifying
	STATE->BindVertexArray(0);
}

//---------------------------------
// SpriteRenderer::CalculateTransform
//
// Recalculates the transformation matrix used by the shader
//
void SpriteRenderer::CalculateTransform()
{
	ivec2 viewPos, viewSize;
	STATE->GetViewport(viewPos, viewSize);

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
