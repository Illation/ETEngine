#pragma once
#include "Uniform.h"


//===================
// Shader Data
//===================


//-------------------------------
// ShaderData::Upload
//
// Upload a uniform in the shader the GPU
//
template<typename T>
bool ShaderData::Upload(uint32 uniform, const T &data) const
{
	// Try finding the uniform
	auto it = m_Uniforms.find(uniform);

	// If we find it, cast to uniform template specialization and upload
	if (it != m_Uniforms.cend())
	{
		I_Uniform* uni = it->second;

		assert(uni->GetType() == typeid(T));

		static_cast<Uniform<T>*>(uni)->Upload(data);
		return true;
	}

	LOG("ShaderData::Upload > Couldn't find uniform!", LogLevel::Warning);
	return false;
}
