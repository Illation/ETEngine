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
	delete m_EmptyTex; m_EmptyTex = nullptr;
}

void SpriteRenderer::Initialize()
{
	m_pShader = ContentManager::Load<ShaderData>("Shaders/PostSprite.glsl");

	STATE->SetShader( m_pShader );
	m_uTransform = glGetUniformLocation( m_pShader->GetProgram(), "uTransform" );

	m_uTexture = glGetUniformLocation( m_pShader->GetProgram(), "uTexture" );
	glUniform1i(m_uTexture, 0);
	m_u3DTexture = glGetUniformLocation( m_pShader->GetProgram(), "u3DTexture" );
	glUniform1i(m_u3DTexture, 1);

	m_uDraw3D = glGetUniformLocation( m_pShader->GetProgram(), "uDraw3D" );
	m_uLayer = glGetUniformLocation( m_pShader->GetProgram(), "uLayer" );

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

	glVertexAttribIPointer(0, (GLint)1, GL_UNSIGNED_INT, (GLsizei)sizeof( SpriteVertex ), (GLvoid*)offsetof( SpriteVertex, TextureId ));
	glVertexAttribPointer(1, (GLint)4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof( SpriteVertex ), (GLvoid*)offsetof( SpriteVertex, TransformData ));
	glVertexAttribPointer(2, (GLint)4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof( SpriteVertex ), (GLvoid*)offsetof( SpriteVertex, TransformData2 ));
	glVertexAttribPointer(3, (GLint)4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof( SpriteVertex ), (GLvoid*)offsetof( SpriteVertex, Color ));

	//unbind
	STATE->BindBuffer(GL_ARRAY_BUFFER, 0);
	STATE->BindVertexArray(0);

	CalculateTransform();

	//Create empty dummy texture
	m_EmptyTex = new TextureData( 1, 1, GL_RGB, GL_RGB, GL_FLOAT );

	m_EmptyTex->Build( (void*)(vec4(1).data.data()) );

	TextureParameters params( true );
	params.minFilter = GL_NEAREST;
	params.magFilter = GL_NEAREST;
	m_EmptyTex->SetParameters( params );

	WINDOW.WindowResizeEvent.AddListener( std::bind( &SpriteRenderer::OnWindowResize, this ) );
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
			m_BufferSize = (uint32)m_Sprites.size() * sizeof( SpriteVertex );
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

	CalculateTransform();
	STATE->SetShader(m_pShader);
	STATE->SetActiveTexture(0);
	glUniformMatrix4fv(m_uTransform, 1, GL_FALSE, etm::valuePtr(m_Transform));

	uint32 batchSize = 1;
	uint32 batchOffset = 0;
	uint32 spriteCount = (uint32)m_Sprites.size();
	for(uint32 i = 0; i < spriteCount; ++i)
	{
		if(i < (spriteCount - 1) && m_Sprites[i].TextureId == m_Sprites[i + 1].TextureId)
		{
			++batchSize;
			continue;
		}

		TextureData* texData = m_Textures[m_Sprites[i].TextureId];
		if (texData->GetTarget() == GL_TEXTURE_2D)
		{
			STATE->SetActiveTexture(0);
			glUniform1i(m_uDraw3D, false);
		}
		else
		{
			STATE->SetActiveTexture(1);
			glUniform1i(m_uDraw3D, true);
			glUniform1f(m_uLayer, m_Layer);
		}
		STATE->BindTexture(texData->GetTarget(), texData->GetHandle());

		//Draw
		glDrawArrays( GL_POINTS, batchOffset, batchSize );
		PERFORMANCE->m_DrawCalls++;

		batchOffset += batchSize;
		batchSize = 1;
	}

	STATE->BindVertexArray( 0 );

	m_Sprites.clear();
	m_Textures.clear();
}

void SpriteRenderer::Draw( TextureData* pTexture, vec2 position, vec4 color /*= vec4(1)*/, vec2 pivot /*= vec2( 0 )*/, 
	vec2 scale /*= vec2( 1 )*/, float rotation /*= 0.f*/, float depth /*= 0.f */, SpriteScalingMode mode /*= SCREEN */, float layer /*= 0.f*/)
{
	SpriteVertex vertex;

	if(pTexture == nullptr)
	{
		pTexture = m_EmptyTex;
	}

	auto it = find( m_Textures.begin(), m_Textures.end(), pTexture );

	if(it == m_Textures.end())
	{
		m_Textures.push_back( pTexture );
		vertex.TextureId = (uint32)m_Textures.size() - 1;
	}
	else
	{
		vertex.TextureId = (uint32)(it - m_Textures.begin());
	}

	vertex.TransformData = vec4( position, depth, rotation );
	switch(mode)
	{
	case SpriteScalingMode::PIXEL :
		//no modification required
		break;
	case SpriteScalingMode::SCREEN :
		{
			ivec2 viewPos, viewSize;
			STATE->GetViewport(viewPos, viewSize);
			scale = scale * etm::vecCast<float>(viewSize);
		}
		break;
	case SpriteScalingMode::TEXTURE :
		scale = scale * vec2( (float)pTexture->GetResolution().x, (float)pTexture->GetResolution().y ) / GRAPHICS.TextureScaleFactor;
		break;
	case SpriteScalingMode::TEXTURE_ABS :
		scale = scale * vec2((float)pTexture->GetResolution().x, (float)pTexture->GetResolution().y);
		break;
	}
	vertex.TransformData2 = vec4( pivot, scale );
	vertex.Color = color;

	if(pTexture->GetTarget() == GL_TEXTURE_3D)m_Layer = layer;

	m_Sprites.push_back( vertex );
}

void SpriteRenderer::CalculateTransform()
{
	ivec2 viewPos, viewSize;
	STATE->GetViewport(viewPos, viewSize);
	int32 width = viewSize.x, height = viewSize.y;
	float scaleX = (width > 0) ? 2.f / width : 0;
	float scaleY = (height > 0) ? 2.f / height : 0;

	m_Transform = mat4({
		scaleX,	0,			0,		0,
		0,		-scaleY,	0,		0,
		0,		0,			1,		0,
		-1,		1,			0,		1 });
}

void SpriteRenderer::OnWindowResize()
{
	CalculateTransform();
}
