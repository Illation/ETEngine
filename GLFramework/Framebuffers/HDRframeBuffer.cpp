#include "stdafx.hpp"
#include "HDRframeBuffer.hpp"

#include "../Graphics/ShaderData.hpp"

HDRframeBuffer::HDRframeBuffer():
	FrameBuffer("Resources/Shaders/HDR.glsl", GL_FLOAT)
{
}
HDRframeBuffer::~HDRframeBuffer()
{
}

void HDRframeBuffer::AccessShaderAttributes()
{
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texFramebuffer"), 0);
	m_uExposure = glGetUniformLocation(m_pShader->GetProgram(), "exposure");
	m_uGamma = glGetUniformLocation(m_pShader->GetProgram(), "gamma");
}

void HDRframeBuffer::UploadDerivedVariables()
{
	glUniform1f(m_uExposure, m_Exposure);
	glUniform1f(m_uGamma, m_Gamma);
}