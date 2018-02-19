#include "stdafx.hpp"
#include "DebugRenderer.h"
#include "CameraComponent.hpp"
#include "ShaderData.hpp"

DebugRenderer::DebugRenderer()
{

}

DebugRenderer::~DebugRenderer()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	m_Lines.clear();
	m_MetaData.clear();
}

void DebugRenderer::Initialize()
{
	m_pShader = ContentManager::Load<ShaderData>("Shaders/DebugRenderer.glsl");

	STATE->SetShader(m_pShader);
	m_uWVP = glGetUniformLocation(m_pShader->GetProgram(), "uViewProj");

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

	glVertexAttribPointer(0, (GLint)3, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(LineVertex), (GLvoid*)offsetof(LineVertex, pos));
	glVertexAttribPointer(1, (GLint)4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(LineVertex), (GLvoid*)offsetof(LineVertex, col));

	//unbind
	STATE->BindBuffer(GL_ARRAY_BUFFER, 0);
	STATE->BindVertexArray(0);
}

void DebugRenderer::UpdateBuffer()
{
	//Bind Object vertex array
	STATE->BindVertexArray(m_VAO);

	//Send the vertex buffer again
	STATE->BindBuffer(GL_ARRAY_BUFFER, m_VBO);

	bool bufferResize = m_Lines.size() * sizeof(LineVertex) > m_BufferSize;
	if (!m_VBO || bufferResize) //first creation or resize
	{
		if (bufferResize)
		{
			m_BufferSize = (uint32)m_Lines.size() * sizeof(LineVertex);
		}

		glBufferData(GL_ARRAY_BUFFER, m_BufferSize, m_Lines.data(), GL_DYNAMIC_DRAW);
	}
	else
	{
		GLvoid* p = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(p, m_Lines.data(), sizeof(LineVertex)*m_Lines.size());
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}


	STATE->BindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugRenderer::Draw()
{
	if (m_Lines.size() <= 0)
		return;

	STATE->SetShader(m_pShader);
	glUniformMatrix4fv(m_uWVP, 1, GL_FALSE, etm::valuePtr(CAMERA->GetViewProj()));

	UpdateBuffer();

	for (const auto& meta : m_MetaData)
	{
		glLineWidth(meta.thickness);
		STATE->DrawArrays(GL_LINES, meta.start, meta.size);
	}

	STATE->BindVertexArray(0);

	m_Lines.clear();
	m_MetaData.clear();
}

void DebugRenderer::CheckMetaData(float thickness)
{
	if (m_MetaData.size() == 0 || !etm::nearEquals(m_MetaData[m_MetaData.size() - 1].thickness, thickness))
	{
		m_MetaData.push_back(LineMetaData());
		m_MetaData[m_MetaData.size() - 1].thickness = thickness;
		if (m_MetaData.size() > 1)
		{
			m_MetaData[m_MetaData.size() - 1].start = m_MetaData[m_MetaData.size() - 2].start + m_MetaData[m_MetaData.size() - 2].size;
		}
	}
	m_MetaData[m_MetaData.size() - 1].size += sizeof(LineVertex) * 2;
}

void DebugRenderer::DrawLine(vec3 start, vec3 end, vec4 col /*= vec4(1)*/, float thickness /*= 1*/)
{
#ifdef EDITOR
	CheckMetaData(thickness);
	m_Lines.push_back(LineVertex(start, col));
	m_Lines.push_back(LineVertex(end, col));
#endif
}

void DebugRenderer::DrawLine(vec3 start, vec4 startCol, vec3 end, vec4 endCol, float thickness /*= 1*/)
{
#ifdef EDITOR
	CheckMetaData(thickness);
	m_Lines.push_back(LineVertex(start, startCol));
	m_Lines.push_back(LineVertex(end, endCol));
#endif
}
