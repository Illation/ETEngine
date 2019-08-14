#pragma once
#include <rttr/type>

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
public:
	ShaderData() = default;
	ShaderData(GLuint shaderProg);
	~ShaderData();

	GLuint const GetProgram() const { return m_ShaderProgram; }
	std::string const& GetName() const { return m_Name; }

	template<typename T>
	bool Upload(uint32 uniform, const T &data)const;
private:
	friend class ShaderAsset;
	friend class ShaderLoader;

	GLuint m_ShaderProgram;
	std::string m_Name;

	std::map<uint32, I_Uniform*> m_Uniforms;
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
	GLuint CompileShader(const std::string &shaderSourceStr, GLenum type);

	bool Precompile(std::string &shaderContent, 
		bool &useGeo, 
		bool &useFrag, 
		std::string &vertSource, 
		std::string &geoSource, 
		std::string &fragSource);

	bool ReplaceInclude(std::string &line);

	bool GetUniformLocations(GLuint shaderProgram, std::map<uint32, I_Uniform*> &uniforms);

	RTTR_ENABLE(Asset<ShaderData, false>)
};


#include "Shader.inl"