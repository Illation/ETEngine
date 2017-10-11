#include "stdafx.hpp"
#include "ShaderData.hpp"

ShaderData::ShaderData(GLuint shaderProg, GLuint vertShader, GLuint geoShader, GLuint fragShader) :
	m_ShaderProgram(shaderProg),
	m_VertexShader(vertShader),
	m_FragmentShader(fragShader),
	m_GeometryShader(geoShader),
	m_UseFS(true),
	m_UseGS(true)
{
}

ShaderData::ShaderData(GLuint shaderProg, GLuint vertShader, GLuint fragShader) :
	m_ShaderProgram(shaderProg),
	m_VertexShader(vertShader),
	m_FragmentShader(fragShader),
	m_GeometryShader(0),
	m_UseFS(true),
	m_UseGS(false)
{
}

ShaderData::ShaderData(GLuint shaderProg, GLuint vertShader) :
	m_ShaderProgram(shaderProg),
	m_VertexShader(vertShader),
	m_FragmentShader(0),
	m_GeometryShader(0),
	m_UseFS(false),
	m_UseGS(false)
{
}

ShaderData::~ShaderData()
{
	glDeleteProgram(m_ShaderProgram);
	if (m_UseGS)glDeleteShader(m_GeometryShader);
	if (m_UseFS)glDeleteShader(m_FragmentShader);
	glDeleteShader(m_VertexShader);
}
