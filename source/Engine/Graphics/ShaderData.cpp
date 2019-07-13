#include "stdafx.h"
#include "ShaderData.h"


ShaderData::ShaderData(GLuint shaderProg) : m_ShaderProgram(shaderProg) { }
ShaderData::~ShaderData()
{
	for (auto &uni : m_Uniforms)
	{
		delete uni.second;
	}
	glDeleteProgram(m_ShaderProgram);
}

void detail::UploadUniform(const Uniform<bool> &uniform)
{
	glUniform1i(uniform.location, uniform.data);
}
void detail::UploadUniform(const Uniform<mat4> &uniform)
{
	glUniformMatrix4fv(uniform.location, 1, GL_FALSE, etm::valuePtr(uniform.data));
}
void detail::UploadUniform(const Uniform<mat3> &uniform)
{
	glUniformMatrix3fv(uniform.location, 1, GL_FALSE, etm::valuePtr(uniform.data));
}
void detail::UploadUniform(const Uniform<vec4> &uniform)
{
	glUniform4f(uniform.location, uniform.data.x, uniform.data.y, uniform.data.z, uniform.data.w);
}
void detail::UploadUniform(const Uniform<vec3> &uniform)
{
	glUniform3f(uniform.location, uniform.data.x, uniform.data.y, uniform.data.z);
}
void detail::UploadUniform(const Uniform<vec2> &uniform) 
{ 
	glUniform2f(uniform.location, uniform.data.x, uniform.data.y);
}
void detail::UploadUniform(const Uniform<float> &uniform)
{
	glUniform1f(uniform.location, uniform.data);
}
void detail::UploadUniform(const Uniform<int32> &uniform)
{
	glUniform1i(uniform.location, uniform.data);
}
