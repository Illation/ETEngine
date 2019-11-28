#pragma once
#include "VertexInfo.h"

#include <EtCore/Content/Asset.h>
#include <EtCore/Helper/LinkerUtils.h>


// forward declarations
class I_Uniform;


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
	std::string const& GetName() const { return m_Name; }
	std::vector<T_AttributeLocation> const& GetAttributes() const { return m_Attributes; }

	// accessors
	//---------------------
	template<typename T>
	bool Upload(T_Hash const uniform, const T &data, bool const reportWarnings = true) const;

	// Data
	///////
private:

	T_ShaderLoc m_ShaderProgram;
	std::string m_Name;

	std::map<T_Hash, I_Uniform*> m_Uniforms;
	std::vector<T_AttributeLocation> m_Attributes;

	T_BlockIndex m_SharedVarIdx;
};

//---------------------------------
// ShaderAsset
//
// Loadable Shader Data
//
class ShaderAsset final : public Asset<ShaderData, false>
{
	DECLARE_FORCED_LINKING()
public:
	// Construct destruct
	//---------------------
	ShaderAsset() : Asset<ShaderData, false>() {}
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

	void GetUniformLocations(T_ShaderLoc const shaderProgram, std::map<uint32, I_Uniform*>& uniforms);
	void GetAttributes(T_ShaderLoc const shaderProgram, std::vector<ShaderData::T_AttributeLocation>& attributes);

	RTTR_ENABLE(Asset<ShaderData, false>)
};


#include "Shader.inl"