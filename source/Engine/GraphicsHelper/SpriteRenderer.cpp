#include "stdafx.hpp"
#include "SpriteRenderer.hpp"
#include "TextureData.hpp"
#include "ShaderData.hpp"

SpriteRenderer::SpriteRenderer()
{
}

SpriteRenderer::~SpriteRenderer()
{
	glDeleteVertexArrays( 1, &m_VAO );
	glDeleteBuffers( 1, &m_VBO );
	m_Sprites.clear();
	m_Textures.clear();
}

void SpriteRenderer::Initialize()
{
	m_pShader = ContentManager::Load<ShaderData>("Shaders/PostSprite.glsl");

	STATE->SetShader( m_pShader );
	m_uTransform = glGetUniformLocation( m_pShader->GetProgram(), "uTransform" );
	m_uTextureSize = glGetUniformLocation( m_pShader->GetProgram(), "uTexSize" );
	m_uTexture = glGetUniformLocation( m_pShader->GetProgram(), "uTexture" );

	glUniform1i(m_uTexture, 0);

	//Generate buffers and arrays
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	//bind
	STATE->BindVertexArray(m_VAO);
	STATE->BindBuffer(GL_ARRAY_BUFFER, m_VBO);

	//set data and attributes
	//glBufferData(GL_ARRAY_BUFFER, m_BufferSize, NULL, GL_DYNAMIC_DRAW);

	//input layout
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	int32 offset = 0;	int32 stride = sizeof( uint32 );
	glVertexAttribIPointer(0, (GLint)1, GL_UNSIGNED_INT, (GLsizei)sizeof( SpriteVertex ), (GLvoid*)offsetof( SpriteVertex, TextureId ));
	offset += stride; stride = 4 * sizeof(GLfloat);
	glVertexAttribPointer(1, (GLint)4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof( SpriteVertex ), (GLvoid*)offsetof( SpriteVertex, TransformData ));
	offset += stride; stride = 4 * sizeof(GLfloat);
	glVertexAttribPointer(2, (GLint)4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof( SpriteVertex ), (GLvoid*)offsetof( SpriteVertex, TransformData2 ));
	offset += stride; stride = 4 * sizeof(GLfloat);
	glVertexAttribPointer(3, (GLint)4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof( SpriteVertex ), (GLvoid*)offsetof( SpriteVertex, Color ));

	//unbind
	STATE->BindBuffer(GL_ARRAY_BUFFER, 0);
	STATE->BindVertexArray(0);

	int32 width = SETTINGS->Window.Width, height = SETTINGS->Window.Height;
	float scaleX = (width > 0) ? 2.f / width : 0;
	float scaleY = (height > 0) ? 2.f / height : 0;

	m_Transform = mat4({
		scaleX,	0,			0,		0,
		0,		-scaleY,	0,		0,
		0,		0,			1,		0,
		-1,		1,			0,		1 });
}

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
			m_BufferSize += m_Sprites.size() * sizeof( SpriteVertex );
		}

		glBufferData(GL_ARRAY_BUFFER, m_BufferSize, m_Sprites.data(), GL_DYNAMIC_DRAW);
	}
	else
	{
		GLvoid* p = glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
		memcpy( p, m_Sprites.data(), sizeof( SpriteVertex ) );
		glUnmapBuffer( GL_ARRAY_BUFFER );
	}


	STATE->BindBuffer(GL_ARRAY_BUFFER, 0);

	//Done Modifying
	STATE->BindVertexArray(0);
}

void SpriteRenderer::Draw()
{
	if(m_Sprites.size() <= 0)
		return;

	UpdateBuffer();

	STATE->BindVertexArray( m_VAO );

	STATE->SetShader(m_pShader);
	STATE->SetActiveTexture(0);
	glUniformMatrix4fv(m_uTransform, 1, GL_FALSE, etm::valuePtr(m_Transform));

	uint32 batchSize = 1;
	uint32 batchOffset = 0;
	uint32 spriteCount = m_Sprites.size();
	for(uint32 i = 0; i < spriteCount; ++i)
	{
		if(i < (spriteCount - 1) && m_Sprites[i].TextureId == m_Sprites[i + 1].TextureId)
		{
			++batchSize;
			continue;
		}

		TextureData* texData = m_Textures[m_Sprites[i].TextureId];
		STATE->BindTexture(GL_TEXTURE_2D, texData->GetHandle());//maybe use lazybind instead

		ivec2 texSize = texData->GetResolution();
		glUniform2f( m_uTextureSize, (float)texSize.x, (float)texSize.y );

		//Draw
		glDrawArrays( GL_TRIANGLES, batchOffset, batchSize );
		PERFORMANCE->m_DrawCalls++;

		batchOffset += batchSize;
		batchSize = 1;
	}

	STATE->BindVertexArray( 0 );
}

void SpriteRenderer::Draw( TextureData* pTexture, vec2 position, vec4 color /*= vec4(1)*/, vec2 pivot /*= vec2( 0 )*/, vec2 scale /*= vec2( 1 )*/, float rotation /*= 0.f*/, float depth /*= 0.f */ )
{
	SpriteVertex vertex;

	auto it = find( m_Textures.begin(), m_Textures.end(), pTexture );

	if(it == m_Textures.end())
	{
		m_Textures.push_back( pTexture );
		vertex.TextureId = m_Textures.size() - 1;
	}
	else
	{
		vertex.TextureId = it - m_Textures.begin();
	}

	vertex.TransformData = vec4( position, depth, rotation );
	vertex.TransformData2 = vec4( pivot, scale );
	vertex.Color = color;

	m_Sprites.push_back( vertex );
}