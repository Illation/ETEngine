#pragma once
#include "VertexInfo.h"
#include "ParameterBlock.h"

#include <EtCore/Content/Asset.h>
#include <EtCore/Util/LinkerUtils.h>


namespace et {
namespace render {


// forward
class TextureData;


//---------------------------------
// ShaderData
//
// Shader that can be used to draw things on the GPU - contains information about it's uniforms
//
class ShaderData final
{
	// definitions
	//---------------------
	friend class ShaderAsset;

public:
	typedef std::pair<T_AttribLoc, AttributeDescriptor> T_AttributeLocation;

	// Construct destruct
	//---------------------
	ShaderData() = default;
	ShaderData(T_ShaderLoc const program);
	~ShaderData();

	// accessors
	//---------------------
	T_ShaderLoc const GetProgram() const { return m_ShaderProgram; }

	std::vector<T_AttributeLocation> const& GetAttributes() const { return m_Attributes; }

	std::vector<render::UniformParam> const& GetUniformLayout() const { return m_UniformLayout; }
	std::vector<core::HashString> const& GetUniformIds() const { return m_UniformIds; }
	render::T_ConstParameterBlock GetCurrentUniforms() const { return m_CurrentUniforms; }

	// functionliaty
	//---------------------
	render::T_ParameterBlock CopyParameterBlock(render::T_ConstParameterBlock const source) const;
	void UploadParameterBlock(render::T_ConstParameterBlock const block) const;

	template<typename TDataType>
	bool Upload(T_Hash const uniform, TDataType const& data, bool const reportWarnings = true) const;

	template<>
	bool Upload<TextureData const*>(T_Hash const uniform, TextureData const* const& textureData, bool const reportWarnings) const;

	// Data
	///////
private:

	T_ShaderLoc m_ShaderProgram;

	// attribute data
	//----------------
	std::vector<T_AttributeLocation> m_Attributes;

	// uniform data
	//--------------

	// loose uniforms
	std::vector<render::UniformParam> m_UniformLayout;
	std::vector<core::HashString> m_UniformIds;
	render::T_ParameterBlock m_CurrentUniforms = nullptr;
	size_t m_UniformDataSize = 0u;

	// within blocks
	std::vector<core::HashString> m_UniformBlocks; // addressed by their indices
};

//---------------------------------
// ShaderAsset
//
// Loadable Shader Data
//
class ShaderAsset final : public core::Asset<ShaderData, false>
{
	DECLARE_FORCED_LINKING()
public:
	// Construct destruct
	//---------------------
	ShaderAsset() : core::Asset<ShaderData, false>() {}
	virtual ~ShaderAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	// Utility
	//---------------------
private:
	T_ShaderLoc CompileShader(std::string const& shaderSourceStr, E_ShaderType const type);

	bool Precompile(std::string &shaderContent, 
		bool &useGeo, 
		bool &useFrag, 
		std::string &vertSource, 
		std::string &geoSource, 
		std::string &fragSource);

	bool ReplaceInclude(std::string &line);

	void InitUniforms();
	void GetAttributes(T_ShaderLoc const shaderProgram, std::vector<ShaderData::T_AttributeLocation>& attributes);

	RTTR_ENABLE(core::Asset<ShaderData, false>)
};


} // namespace render
} // namespace et


#include "Shader.inl"
