#include "stdafx.h"
#include "Uniform.h"

#include <glad/glad.h>


//===================
// Uniform
//===================


namespace detail {


//---------------------------------
// UploadUniform
//
// Upload a boolean to the GPU
//
void UploadUniform(const Uniform<bool> &uniform)
{
	glUniform1i(uniform.location, uniform.data);
}

//---------------------------------
// UploadUniform
//
// Upload a 4x4 Matrix to the GPU
//
void UploadUniform(const Uniform<mat4> &uniform)
{
	glUniformMatrix4fv(uniform.location, 1, GL_FALSE, etm::valuePtr(uniform.data));
}

//---------------------------------
// UploadUniform
//
// Upload a 3x3 Matrix to the GPU
//
void UploadUniform(const Uniform<mat3> &uniform)
{
	glUniformMatrix3fv(uniform.location, 1, GL_FALSE, etm::valuePtr(uniform.data));
}

//---------------------------------
// UploadUniform
//
// Upload a 4D Vector to the GPU
//
void UploadUniform(const Uniform<vec4> &uniform)
{
	glUniform4f(uniform.location, uniform.data.x, uniform.data.y, uniform.data.z, uniform.data.w);
}

//---------------------------------
// UploadUniform
//
// Upload a 3D Vector to the GPU
//
void UploadUniform(const Uniform<vec3> &uniform)
{
	glUniform3f(uniform.location, uniform.data.x, uniform.data.y, uniform.data.z);
}

//---------------------------------
// UploadUniform
//
// Upload a 2D Vector to the GPU
//
void UploadUniform(const Uniform<vec2> &uniform)
{
	glUniform2f(uniform.location, uniform.data.x, uniform.data.y);
}

//---------------------------------
// UploadUniform
//
// Upload a scalar to the GPU
//
void UploadUniform(const Uniform<float> &uniform)
{
	glUniform1f(uniform.location, uniform.data);
}

//---------------------------------
// UploadUniform
//
// Upload an integer to the GPU
//
void UploadUniform(const Uniform<int32> &uniform)
{
	glUniform1i(uniform.location, uniform.data);
}

//---------------------------------
// UploadUniform
//
// Upload an unsigned integer to the GPU
//
void UploadUniform(const Uniform<uint32> &uniform)
{
	glUniform1ui(uniform.location, uniform.data);
}

//---------------------------------
// InitUniform
//
// Download a boolean from the GPU
//
void InitUniform(uint32 const program, Uniform<bool> &uniform)
{
	glGetUniformiv(program, uniform.location, reinterpret_cast<int32*>(&uniform.data));
}

//---------------------------------
// InitUniform
//
// Download an integer from the GPU
//
void InitUniform(uint32 const program, Uniform<int32> &uniform)
{
	glGetUniformiv(program, uniform.location, &uniform.data);
}

//---------------------------------
// InitUniform
//
// Download an unsigned integer from the GPU
//
void InitUniform(uint32 const program, Uniform<uint32> &uniform)
{
	glGetUniformuiv(program, uniform.location, &uniform.data);
}

//---------------------------------
// InitUniform
//
// Download a float from the GPU
//
void InitUniform(uint32 const program, Uniform<float> &uniform)
{
	glGetUniformfv(program, uniform.location, &uniform.data);
}

//---------------------------------
// InitUniform
//
// Download a 2D vector from the GPU
//
void InitUniform(uint32 const program, Uniform<vec2> &uniform)
{
	glGetUniformfv(program, uniform.location, etm::valuePtr(uniform.data));
}

//---------------------------------
// InitUniform
//
// Download a 3D vector from the GPU
//
void InitUniform(uint32 const program, Uniform<vec3> &uniform)
{
	glGetUniformfv(program, uniform.location, etm::valuePtr(uniform.data));
}

//---------------------------------
// InitUniform
//
// Download a 4D vector from the GPU
//
void InitUniform(uint32 const program, Uniform<vec4> &uniform)
{
	glGetUniformfv(program, uniform.location, etm::valuePtr(uniform.data));
}

//---------------------------------
// InitUniform
//
// Download a 3x3 matrix from the GPU
//
void InitUniform(uint32 const program, Uniform<mat3> &uniform)
{
	glGetUniformfv(program, uniform.location, etm::valuePtr(uniform.data));
}

//---------------------------------
// InitUniform
//
// Download a 4x4 matrix from the GPU
//
void InitUniform(uint32 const program, Uniform<mat4> &uniform)
{
	glGetUniformfv(program, uniform.location, etm::valuePtr(uniform.data));
}


} // namespace detail