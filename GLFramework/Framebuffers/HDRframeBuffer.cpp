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
	m_uExposure = glGetUniformLocation(m_pShader->GetProgram(), "exposure");
}

void HDRframeBuffer::UploadDerivedVariables()
{
	glUniform1f(m_uExposure, m_Exposure);
}