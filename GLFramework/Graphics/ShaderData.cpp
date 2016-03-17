#include "stdafx.hpp"
#include "ShaderData.hpp"

ShaderData::ShaderData(GLuint shaderProg, GLuint vertShader, GLuint fragShader) :
	m_VertexShader(vertShader),
	m_GeometryShader(0),
	m_FragmentShader(fragShader),
	m_ShaderProgram(shaderProg),
	m_UseGS(false)
{
}

ShaderData::ShaderData(GLuint shaderProg, GLuint vertShader, GLuint geoShader, GLuint fragShader) :
	m_VertexShader(vertShader),
	m_GeometryShader(geoShader),
	m_FragmentShader(fragShader),
	m_ShaderProgram(shaderProg),
	m_UseGS(true)
{
}

ShaderData::~ShaderData()
{
	glDeleteProgram(m_ShaderProgram);
	glDeleteShader(m_FragmentShader);
	if (m_UseGS)glDeleteShader(m_GeometryShader);
	glDeleteShader(m_VertexShader);
}
