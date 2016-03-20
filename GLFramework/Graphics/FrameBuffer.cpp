#include "stdafx.hpp"

#include "FrameBuffer.hpp"

#include "ShaderData.hpp"
#include "TextureData.hpp"

FrameBuffer::FrameBuffer(string shaderFile, GLenum format, unsigned numTargets)
	:m_ShaderFile(shaderFile),
	m_Format(format),
	m_NumTargets(numTargets)
{
}
FrameBuffer::~FrameBuffer()
{
	glDeleteRenderbuffers(1, &m_RboDepthStencil);
	for (size_t i = 0; i < m_pTextureVec.size(); i++)
	{
		SafeDelete(m_pTextureVec[i]);
	}
	glDeleteFramebuffers(1, &m_GlFrameBuffer);

	glDeleteBuffers(1, &m_VertexBufferObject);
	glDeleteVertexArrays(1, &m_VertexArrayObject);
}

void FrameBuffer::Initialize()
{
	std::cout << "Initializing Frame Buffer . . .";
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
	m_pShader = ContentManager::Load<ShaderData>(m_ShaderFile);

	//Specify Input Layouts
	glBindVertexArray(m_VertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferObject);
	DefAttLayout(m_pShader->GetProgram());

	//GetAccessTo shader attributes
	glUseProgram(m_pShader->GetProgram());
	AccessShaderAttributes();

	//FrameBuffer
	glGenFramebuffers(1, &m_GlFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_GlFrameBuffer);//Bind

	int width = SETTINGS->Window.Width, height = SETTINGS->Window.Height;
	vector<GLuint> attachments;
	attachments.reserve(m_NumTargets);
	m_pTextureVec.reserve(m_NumTargets);
	for (size_t i = 0; i < m_NumTargets; i++)
	{
		GLuint texHandle;
		glGenTextures(1, &texHandle);
		glBindTexture(GL_TEXTURE_2D, texHandle);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, m_Format, NULL
			);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texHandle, 0
			);
		m_pTextureVec.push_back(new TextureData(texHandle, width, height));
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}


	//Render Buffer for depth and stencil
	glGenRenderbuffers(1, &m_RboDepthStencil);
	glBindRenderbuffer(GL_RENDERBUFFER, m_RboDepthStencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SETTINGS->Window.Width, SETTINGS->Window.Height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RboDepthStencil);

	glDrawBuffers(m_NumTargets, attachments.data());

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) std::cout << "  . . . SUCCESS!" << std::endl;
	else std::cout << "  . . . FAILED!" << std::endl;
}

void FrameBuffer::AccessShaderAttributes()
{
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texFramebuffer"), 0);
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

	if (m_NumTargets == 4)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_pTextureVec[0]->GetHandle());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_pTextureVec[1]->GetHandle());
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_pTextureVec[2]->GetHandle());
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_pTextureVec[3]->GetHandle());
	}
	else
	{
		for (size_t i = 0; i < m_NumTargets; i++)
		{
			glActiveTexture(GL_TEXTURE0+i);
			glBindTexture(GL_TEXTURE_2D, m_pTextureVec[i]->GetHandle());
		}
	}

	UploadDerivedVariables();

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