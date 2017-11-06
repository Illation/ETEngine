#include "stdafx.hpp"
#include "SpriteRenderer.hpp"

SpriteRenderer::SpriteRenderer()
{
}

SpriteRenderer::~SpriteRenderer()
{
	m_Sprites.clear();
	m_Textures.clear();
}

void SpriteRenderer::Initialize()
{
	m_pShader = ContentManager::Load<ShaderData>("Shaders/PostSprite.glsl");

	STATE->SetShader(m_pTextShader);
	m_uTransform = glGetUniformLocation( m_pShader->GetProgram(), "transform" );
	m_uTextureSize = glGetUniformLocation( m_pShader->GetProgram(), "textureSize" );
	m_uTexture = glGetUniformLocation( m_pShader->GetProgram(), "texture" );

	glUniform1i(m_uTexture, 0);

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
