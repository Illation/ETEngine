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
bool ShaderData::Upload(T_Hash const uniform, const T &data, bool const reportWarnings) const
{
	// Try finding the uniform
	auto const it = m_Uniforms.find(uniform);

	// If we find it, cast to uniform template specialization and upload
	if (it != m_Uniforms.cend())
	{
		I_Uniform* uni = it->second;

		assert(uni->GetType() == typeid(T));

		static_cast<Uniform<T>*>(uni)->Upload(data);
		return true;
	}

	if (reportWarnings)
	{
		LOG("ShaderData::Upload > Couldn't find uniform!", LogLevel::Warning);
	}

	return false;
}
