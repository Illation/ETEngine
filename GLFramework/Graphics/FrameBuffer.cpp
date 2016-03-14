#include "FrameBuffer.hpp"

#include <iostream>
#include "../Content/ShaderLoader.hpp"
#include "../Content/ContentManager.hpp"
#include "ShaderData.hpp"

#include "../Base\Settings.hpp"
#define SETTINGS Settings::GetInstance()

FrameBuffer::FrameBuffer()
{
}


FrameBuffer::~FrameBuffer()
{
	glDeleteRenderbuffers(1, &m_RboDepthStencil);
	glDeleteTextures(1, &m_TexColBuffer);
	glDeleteFramebuffers(1, &m_GlFrameBuffer);

	glDeleteBuffers(1, &m_VertexBufferObject);
	glDeleteVertexArrays(1, &m_VertexArrayObject);
}

void FrameBuffer::Initialize()
{
	GLfloat quadVertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f,  1.0f,  1.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f
	};

	//Vertex Array Object
	glGenVertexArrays(1, &m_VertexArrayObject);
	//Vertex Buffer Object
	glGenBuffers(1, &m_VertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	//Load and compile Shaders
	m_pShader = ContentManager::Load<ShaderData>("Resources/outlineEffect.glsl");

	//Specify Input Layouts
	glBindVertexArray(m_VertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferObject);
	DefAttLayout(m_pShader->GetProgram());

	//GetAccessTo shader attributes
	glUseProgram(m_pShader->GetProgram());
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texFramebuffer"), 0);

	//FrameBuffer
	std::cout << "Initializing Frame Buffer . . .";
	glGenFramebuffers(1, &m_GlFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_GlFrameBuffer);//Bind

	glGenTextures(1, &m_TexColBuffer);
	glBindTexture(GL_TEXTURE_2D, m_TexColBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SETTINGS->Window.Width, SETTINGS->Window.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TexColBuffer, 0);//Bind Framebuffe to texture
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) std::cout << "  . . . SUCCESS!" << std::endl;
	else std::cout << "  . . . FAILED!" << std::endl;

	//Render Buffer for depth and stencil
	glGenRenderbuffers(1, &m_RboDepthStencil);
	glBindRenderbuffer(GL_RENDERBUFFER, m_RboDepthStencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SETTINGS->Window.Width, SETTINGS->Window.Height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RboDepthStencil);
}

void FrameBuffer::Enable(bool active)
{
	if(active) glBindFramebuffer(GL_FRAMEBUFFER, m_GlFrameBuffer);
	else glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Draw()
{
	glBindVertexArray(m_VertexArrayObject);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(m_pShader->GetProgram());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TexColBuffer);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void FrameBuffer::DefAttLayout(GLuint shaderProgram)
{
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
}