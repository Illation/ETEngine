#include "stdafx.h"

#include "Shader.h"
#include "Uniform.h"

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Content/AssetStub.h>

#include <rttr/registration>
#include <rttr/detail/policies/ctor_policies.h>


//===================
// Shader Data
//===================


// Construct destruct
///////////////

//---------------------------------
// ShaderData::ShaderData
//
// Construct shader data from an OpenGl program pointer
//
ShaderData::ShaderData(GLuint shaderProg) 
	: m_ShaderProgram(shaderProg) 
{ }

//---------------------------------
// ShaderData::~ShaderData
//
// Shader data destructor
//
ShaderData::~ShaderData()
{
	for (auto &uni : m_Uniforms)
	{
		delete uni.second;
	}
	STATE->DeleteProgram(m_ShaderProgram);
}


//===================
// Shader Asset
//===================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<ShaderAsset>("shader asset")
		.constructor<ShaderAsset const&>()
		.constructor<>()( rttr::detail::as_object() );
	rttr::type::register_converter_func( [](ShaderAsset& shader, bool& ok) -> I_Asset*
	{
		ok = true;
		return new ShaderAsset(shader);
	});
}
DEFINE_FORCED_LINKING(ShaderAsset) // force the shader asset class to be linked as it is only used in reflection

//---------------------------------
// ShaderAsset::LoadFromMemory
//
// Load shader data from binary asset content
//
bool ShaderAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	// Extract the shader text from binary data
	//------------------------
	std::string shaderContent = FileUtil::AsText(data);
	if (shaderContent.size() == 0)
	{
		LOG("ShaderAsset::LoadFromMemory > Empty shader file!", Warning);
		return false;
	}

	// Precompile
	//--------------------
	bool useGeo = false;
	bool useFrag = false;
	std::string vertSource;
	std::string geoSource;
	std::string fragSource;
	if (!Precompile(shaderContent, useGeo, useFrag, vertSource, geoSource, fragSource))
	{
		return false;
	}

	// Compile
	//------------------
	GLuint vertexShader = CompileShader(vertSource, GL_VERTEX_SHADER);

	GLuint geoShader = 0;
	if (useGeo)
	{
		geoShader = CompileShader(geoSource, GL_GEOMETRY_SHADER);
	}

	GLuint fragmentShader = 0;
	if (useFrag)
	{
		fragmentShader = CompileShader(fragSource, GL_FRAGMENT_SHADER);
	}

	// Combine Shaders into a program
	//------------------
	GLuint shaderProgram = STATE->CreateProgram();

	STATE->AttachShader(shaderProgram, vertexShader);

	if (useGeo)
	{
		STATE->AttachShader(shaderProgram, geoShader);
	}

	if (useFrag)
	{
		STATE->AttachShader(shaderProgram, fragmentShader);
		STATE->BindFragmentDataLocation(shaderProgram, 0, "outColor");
	}

	STATE->LinkProgram(shaderProgram);

	// Delete shader objects now that we have a program
	STATE->DeleteShader(vertexShader);

	if (useGeo)
	{
		STATE->DeleteShader(geoShader);
	}

	if (useFrag)
	{
		STATE->DeleteShader(fragmentShader);
	}

	// Create shader data
	m_Data = new ShaderData(shaderProgram);

	// Extract uniform info
	//------------------
	STATE->SetShader(m_Data);

	std::map<uint32, I_Uniform*> uniforms;
	GetUniformLocations(shaderProgram, uniforms);

	m_Data->m_Uniforms = uniforms;

	// all done
	return true;
}

//---------------------------------
// ShaderAsset::CompileShader
//
// Compile a glsl shader
//
GLuint ShaderAsset::CompileShader(std::string const&shaderSourceStr, GLenum type)
{
	const char *shaderSource = shaderSourceStr.c_str();
	GLuint shader = STATE->CreateShader(type);
	STATE->SetShaderSource(shader, 1, &shaderSource, nullptr);

	//error handling
	GLint status;
	STATE->CompileShader(shader);
	STATE->GetShaderIV(shader, GL_COMPILE_STATUS, &status);
	if (!(status == GL_TRUE))
	{
		char buffer[512];
		STATE->GetShaderInfoLog(shader, 512, NULL, buffer);
		std::string sName;
		switch (type)
		{
		case GL_VERTEX_SHADER:
			sName = "vertex";
			break;
		case GL_GEOMETRY_SHADER:
			sName = "geometry";
			break;
		case GL_FRAGMENT_SHADER:
			sName = "fragment";
			break;
		default:
			sName = "invalid type";
			break;
		}
		LOG(std::string("ShaderAsset::CompileShader > Compiling ") + sName + " shader failed", LogLevel::Warning);
		LOG(buffer, Warning);
	}

	return shader;
}

//---------------------------------
// ShaderAsset::Precompile
//
// Precompile a .glsl file into multiple shader strings that can be compiled on their own
//
bool ShaderAsset::Precompile(std::string &shaderContent, 
	bool &useGeo, 
	bool &useFrag, 
	std::string &vertSource, 
	std::string &geoSource, 
	std::string &fragSource)
{
	enum ParseState {
		INIT,
		VERT,
		GEO,
		FRAG
	} state = ParseState::INIT;

	std::string extractedLine;
	while (FileUtil::ParseLine(shaderContent, extractedLine))
	{
		//Includes
		if (extractedLine.find("#include") != std::string::npos)
		{
			if (!(ReplaceInclude(extractedLine)))
			{
				LOG(std::string("ShaderAsset::Precompile > Replacing include at '") + extractedLine + "' failed!", LogLevel::Warning);
				return false;
			}
		}

		//Precompile types
		switch (state)
		{
		case INIT:
			if (extractedLine.find("<VERTEX>") != std::string::npos)
			{
				state = ParseState::VERT;
			}
			if (extractedLine.find("<GEOMETRY>") != std::string::npos)
			{
				useGeo = true;
				state = ParseState::GEO;
			}
			if (extractedLine.find("<FRAGMENT>") != std::string::npos)
			{
				state = ParseState::FRAG;
				useFrag = true;
			}
			break;
		case VERT:
			if (extractedLine.find("</VERTEX>") != std::string::npos)
			{
				state = ParseState::INIT;
				break;
			}
			vertSource += extractedLine;
			vertSource += "\n";
			break;
		case GEO:
			if (extractedLine.find("</GEOMETRY>") != std::string::npos)
			{
				state = ParseState::INIT;
				break;
			}
			geoSource += extractedLine;
			geoSource += "\n";
			break;
		case FRAG:
			if (extractedLine.find("</FRAGMENT>") != std::string::npos)
			{
				state = ParseState::INIT;
				break;
			}
			else if (extractedLine.find("#disable") != std::string::npos)
			{
				useFrag = false;
				state = ParseState::INIT;
				break;
			}
			fragSource += extractedLine;
			fragSource += "\n";
			break;
		}
	}

	return true;
}

//---------------------------------
// ShaderAsset::ReplaceInclude
//
// Include another shader asset inline
//
bool ShaderAsset::ReplaceInclude(std::string &line)
{
	// Get the asset ID
	uint32 firstQ = (uint32)line.find("\"");
	uint32 lastQ = (uint32)line.rfind("\"");
	if ((firstQ == std::string::npos) ||
		(lastQ == std::string::npos) ||
		lastQ <= firstQ)
	{
		LOG(std::string("ShaderAsset::ReplaceInclude > Replacing include line '") + line + "' failed", LogLevel::Warning);
		return false;
	}
	firstQ++;
	std::string path = line.substr(firstQ, lastQ - firstQ);
	T_Hash const assetId(GetHash(FileUtil::ExtractName(path)));

	// Get the stub asset data
	auto const foundRefIt = std::find_if(GetReferences().cbegin(), GetReferences().cend(), [assetId](Reference const& reference)
	{
		ET_ASSERT(reference.GetAsset() != nullptr);
		return reference.GetAsset()->GetAsset()->GetId() == assetId;
	});

	if (foundRefIt == GetReferences().cend())
	{
		LOG(std::string("ShaderAsset::ReplaceInclude > Asset at path '") + path + "' not found in references!", LogLevel::Warning);
		return false;
	}
	I_AssetPtr const* const rawAssetPtr = foundRefIt->GetAsset();
	ET_ASSERT(rawAssetPtr->GetType() == typeid(StubData), "Asset reference found at path %s is not of type StubData", path);
	AssetPtr<StubData> stubPtr = *static_cast<AssetPtr<StubData> const*>(rawAssetPtr);

	// extract the shader string
	std::string shaderContent(stubPtr->GetText(), stubPtr->GetLength());
	if (shaderContent.size() == 0)
	{
		LOG(std::string("ShaderAsset::ReplaceInclude > Shader string extracted from stub data at'") + path + "' was empty!", LogLevel::Warning);
		return false;
	}

	// replace the original line with the included shader
	line = "";
	std::string extractedLine;
	while (FileUtil::ParseLine(shaderContent, extractedLine))
	{
		//Includes
		if (extractedLine.find("#include") != std::string::npos)
		{
			if (!(ReplaceInclude(extractedLine)))
			{
				LOG(std::string("ShaderAsset::ReplaceInclude > Replacing include at '") + extractedLine + "' failed!", LogLevel::Warning);
				return false;
			}
		}

		line += extractedLine + "\n";
	}

	// we're done
	return true;
}

//---------------------------------
// ShaderAsset::GetUniformLocations
//
// Extract shader uniforms from a program
//
bool ShaderAsset::GetUniformLocations(GLuint shaderProgram, std::map<uint32, I_Uniform*> &uniforms)
{
	GLint count;
	STATE->GetProgramIV(shaderProgram, GL_ACTIVE_UNIFORMS, &count);
	for (GLint i = 0; i < count; i++)
	{
		GLint size;
		GLenum type;

		const GLsizei bufSize = 256;
		GLchar name[bufSize];
		GLsizei length;

		STATE->GetActiveUniform(shaderProgram, (GLuint)i, bufSize, &length, &size, &type, name);
		std::string uniName = std::string(name, length);
		std::string endName;

		if (size > 1)
		{
			auto found = uniName.find(']');
			if (found < uniName.size() - 1) endName = uniName.substr(uniName.find(']') + 1);
			uniName = uniName.substr(0, uniName.find('['));
		}

		for (uint32 j = 0; j < (uint32)size; ++j)
		{
			std::string fullName = uniName;
			if (size > 1)fullName += "[" + std::to_string(j) + "]" + endName;

			I_Uniform* pUni;
			switch (type)
			{
			case GL_BOOL:
				pUni = new Uniform<bool>();
				break;
			case GL_INT:
				pUni = new Uniform<int32>();
				break;
			case GL_UNSIGNED_INT:
				pUni = new Uniform<uint32>();
				break;
			case GL_FLOAT:
				pUni = new Uniform<float>();
				break;
			case GL_FLOAT_VEC2:
				pUni = new Uniform<vec2>();
				break;
			case GL_FLOAT_VEC3:
				pUni = new Uniform<vec3>();
				break;
			case GL_FLOAT_VEC4:
				pUni = new Uniform<vec4>();
				break;
			case GL_FLOAT_MAT3:
				pUni = new Uniform<mat3>();
				break;
			case GL_FLOAT_MAT4:
				pUni = new Uniform<mat4>();
				break;
			case GL_SAMPLER_2D:
				pUni = new Uniform<int32>();
				break;
			case GL_SAMPLER_3D:
				pUni = new Uniform<int32>();
				break;
			case GL_SAMPLER_CUBE:
				pUni = new Uniform<int32>();
				break;
			default:
				LOG(std::string("unknown uniform type ") + std::to_string(type), Warning);
				return false;
				break;
			}
			pUni->name = fullName;
			pUni->location = i;
			T_Hash hash = GetHash(fullName);
			assert(uniforms.find(hash) == uniforms.end());
			uniforms[hash] = pUni;
		}
	}
	return true;
}