#pragma once
#include "TextureData.h"

#include <EtRendering/GraphicsContext/Viewport.h>


namespace et {
namespace render {


//===================
// Shader Data
//===================


//-------------------------------
// ShaderData::Upload
//
// Upload a uniform in the shader the GPU for standard parameter types
//
template<typename TDataType>
bool ShaderData::Upload(T_Hash const uniform, const TDataType &data, bool const reportWarnings) const
{
	// Try finding the uniform
	auto const idIt = std::find(m_UniformIds.cbegin(), m_UniformIds.cend(), uniform);

	if (idIt == m_UniformIds.cend())
	{
		if (reportWarnings)
		{
			ET_ASSERT(false, "Couldn't find uniform!");
		}

		return false;
	}

	render::UniformParam const& param = m_UniformLayout[idIt - m_UniformIds.cbegin()];

	if (render::parameters::Read<TDataType>(m_CurrentUniforms, param.offset) == data)
	{
		return true; // no need for API call as the state wouldn't change
	}

	ET_ASSERT(render::parameters::GetTypeId(param.type) == rttr::type::get<TDataType>());

	Viewport::GetCurrentApiContext()->UploadUniform(param.location, data);

	// ensure the shader reflects the GPU state
	render::parameters::Write<TDataType>(m_CurrentUniforms, param.offset, data);

	return true;
}

//-------------------------------
// ShaderData::Upload
//
// Upload a texture to a shader
//
template<>
bool ShaderData::Upload<TextureData const*>(T_Hash const uniform, TextureData const* const& textureData, bool const reportWarnings) const
{
	// Try finding the uniform
	auto const idIt = std::find(m_UniformIds.cbegin(), m_UniformIds.cend(), uniform);

	if (idIt == m_UniformIds.cend())
	{
		if (reportWarnings)
		{
			ET_ASSERT(false, "Couldn't find uniform!");
		}

		return false;
	}

	render::UniformParam const& param = m_UniformLayout[idIt - m_UniformIds.cbegin()];

	ET_ASSERT(render::parameters::MatchesTexture(param.type, textureData->GetTargetType()));

	//T_TextureHandle const texHandle = textureData->GetHandle();
	//if (texHandle != 0u)
	//{
	//	if (render::parameters::Read<TextureData const*>(m_CurrentUniforms, param.offset) == textureData)
	//	{
	//		return true; // no need for API call as the state wouldn't change
	//	}

	//	ET_ASSERT(false, "Uploading bindless textures is not yet supported!");
	//	// #todo: upload bindless texture handle
	//}
	//else
	//{
		// we overwrite sampler uniforms every time because the bindings may have changed for bound textures
		I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

		T_TextureUnit const binding = api->BindTexture(textureData->GetTargetType(), textureData->GetLocation(), false);
		api->UploadUniform(param.location, static_cast<int32>(binding));
	//}

	// ensure the shader reflects the GPU state
	render::parameters::Write<TextureData const*>(m_CurrentUniforms, param.offset, textureData);

	return true;
}


} // namespace render
} // namespace et
