#include "stdafx.hpp"
#include "HDRframeBuffer.hpp"

#include "../Graphics/ShaderData.hpp"

HDRframeBuffer::HDRframeBuffer():
	FrameBuffer("Resources/Shaders/PostHDR.glsl", GL_FLOAT, 2)
{
}
HDRframeBuffer::~HDRframeBuffer()
{
}

void HDRframeBuffer::AccessShaderAttributes()
{
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texColor"), 0);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texBright"), 1);
	m_uExposure = glGetUniformLocation(m_pShader->GetProgram(), "exposure");
	m_uGamma = glGetUniformLocation(m_pShader->GetProgram(), "gamma");
}

void HDRframeBuffer::UploadDerivedVariables()
{
	glUniform1f(m_uExposure, m_Exposure);
	glUniform1f(m_uGamma, m_Gamma);
}