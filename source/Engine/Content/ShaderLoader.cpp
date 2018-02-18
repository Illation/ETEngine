#include "stdafx.hpp"
#include "ShaderLoader.hpp"

#include "../Graphics/ShaderData.hpp"
#include "FileSystem/Entry.h"
#include <algorithm>
#include "FileSystem/FileUtil.h"
#include "Helper/Hash.h"
#include <functional>
#include <cctype>
#include <locale>

ShaderLoader::ShaderLoader()
{
}


ShaderLoader::~ShaderLoader()
{
}

ShaderData* ShaderLoader::LoadContent(const std::string& assetFile)
{
	logPos = Logger::GetCursorPosition();
	loadingString = std::string("Loading Shader: ") + assetFile + " . . .";

	LOG(loadingString + " . . . reading file          ", Info, false, logPos);

	File* input = new File( assetFile, nullptr );
	if(!input->Open( FILE_ACCESS_MODE::Read ))
	{
		LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
		LOG( "    Opening shader file failed." , Warning);
		return nullptr;
	}
	std::string shaderContent = FileUtil::AsText(input->Read());
	delete input; 
	input = nullptr;
	if(shaderContent.size() == 0)
	{
		LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
		LOG( "    Shader file is empty." , Warning);
		return nullptr;
	}

	//Precompile
	LOG(loadingString + " . . . precompiling          ", Info, false, logPos);
	bool useGeo = false;
	bool useFrag = false;
	std::string vertSource;
	std::string geoSource;
	std::string fragSource;
	if(!Precompile( shaderContent, assetFile, useGeo, useFrag, vertSource, geoSource, fragSource ))
	{
		return nullptr;
	}

	std::map<uint32, AbstractUniform*> uniforms;
	//Compile
	LOG(loadingString + " . . . compiling vertex          ", Info, false, logPos);
	GLuint vertexShader = CompileShader(vertSource, GL_VERTEX_SHADER);

	GLuint geoShader = 0;
	if (useGeo)
	{
		LOG(loadingString + " . . . compiling geometry          ", Info, false, logPos);
		geoShader = CompileShader(geoSource, GL_GEOMETRY_SHADER);
	}

	GLuint fragmentShader = 0;
	if (useFrag)
	{
		LOG(loadingString + " . . . compiling fragment          ", Info, false, logPos);
		fragmentShader = CompileShader(fragSource, GL_FRAGMENT_SHADER);
	}

	//Combine Shaders
	LOG(loadingString + " . . . linking program         ", Info, false, logPos);
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	if(useGeo)glAttachShader(shaderProgram, geoShader);
	if (useFrag)glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");

	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	if (useGeo)glDeleteShader(geoShader);
	if (useFrag)glDeleteShader(fragmentShader);

	LOG(loadingString + " . . . getting uniforms          ", Info, false, logPos);
	GetUniformLocations(shaderProgram, uniforms);

	ShaderData* pShaderData = new ShaderData(shaderProgram);
	pShaderData->m_Name = assetFile;
	pShaderData->m_Uniforms = uniforms;

	LOG(loadingString + " . . . SUCCESS!          ", Info, false, logPos);
	return pShaderData;
}

GLuint ShaderLoader::CompileShader(const std::string &shaderSourceStr, GLenum type)
{
	using namespace std;
	const char *shaderSource = shaderSourceStr.c_str();
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &shaderSource, NULL);

	//error handling
	GLint status;
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!(status == GL_TRUE))
	{
		char buffer[512];
		glGetShaderInfoLog(shader, 512, NULL, buffer);
		LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
		string sName;
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
		LOG(std::string("    Compiling ") + sName + " shader failed", Warning);
		LOG(buffer, Warning);
	}

	return shader;
}

bool ShaderLoader::Precompile(std::string &shaderContent, const std::string &assetFile, bool &useGeo, bool &useFrag, std::string &vertSource, std::string &geoSource, std::string &fragSource)
{
	enum ParseState {
		INIT,
		VERT,
		GEO,
		FRAG
	} state = ParseState::INIT;

	std::string extractedLine;
	while(FileUtil::ParseLine( shaderContent, extractedLine ))
	{
		//Includes
		if (extractedLine.find("#include") != std::string::npos)
		{
			LOG(loadingString + " . . . replacing include          ", Info, false, logPos);
			if (!(ReplaceInclude(extractedLine, assetFile)))
			{
				LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
				LOG( "    Opening shader file failed." , Warning);
				return false;
			}
			LOG(loadingString + " . . . precompiling          ", Info, false, logPos);
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

bool ShaderLoader::ReplaceInclude(std::string &line, const std::string &assetFile)
{
	std::string basePath = "";
	uint32 lastFS = (uint32)assetFile.rfind("/");
	uint32 lastBS = (uint32)assetFile.rfind("\\");
	uint32 lastS = static_cast<uint32>(std::max((int32)lastFS, (int32)lastBS));
	if(!(lastS == std::string::npos))basePath = assetFile.substr(0, lastS) + "/";

	uint32 firstQ = (uint32)line.find("\"");
	uint32 lastQ = (uint32)line.rfind("\"");
	if ((firstQ == std::string::npos) ||
		(lastQ == std::string::npos) ||
		lastQ <= firstQ)
	{
		LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
		LOG( "    invalid include syntax." , Warning);
		LOG(line, Warning);
		return false;
	}
	firstQ++;
	std::string path = basePath + line.substr(firstQ, lastQ - firstQ);

	File* input = new File( path, nullptr );
	if(!input->Open( FILE_ACCESS_MODE::Read ))
	{
		LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
		LOG( "    Opening shader file failed." , Warning);
		return false;
	}
	std::string shaderContent = FileUtil::AsText(input->Read());
	delete input;
	input = nullptr;
	if(shaderContent.size() == 0)
	{
		LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
		LOG( "    Shader file is empty." , Warning);
		return false;
	}
	line = "";
	std::string extractedLine;
	while(FileUtil::ParseLine( shaderContent, extractedLine ))
	{
		//Includes
		if (extractedLine.find("#include") != std::string::npos)
		{
			if (!(ReplaceInclude(extractedLine, assetFile)))
			{
				LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
				LOG( "    Opening shader file failed." , Warning);
				return false;
			}
		}
		
		line += extractedLine + "\n";
	}
	return true;
}

bool ShaderLoader::GetUniformLocations(GLuint shaderProgram, std::map<uint32, AbstractUniform*> &uniforms)
{
	GLint count;
	glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &count);
	for (GLint i = 0; i < count; i++)
	{
		GLint size; 
		GLenum type; 

		const GLsizei bufSize = 256; 
		GLchar name[bufSize]; 
		GLsizei length; 

		glGetActiveUniform(shaderProgram, (GLuint)i, bufSize, &length, &size, &type, name);
		std::string uniName = std::string(name, length);
		std::string endName;
		if (size > 1)
		{
			auto found = uniName.find(']');
			if(found < uniName.size()-1) endName = uniName.substr(uniName.find(']') + 1);
			uniName = uniName.substr(0, uniName.find('['));
		}
		for (uint32 j = 0; j < (uint32)size; ++j)
		{
			std::string fullName = uniName;
			if (size > 1)fullName += "[" + std::to_string(j) + "]" + endName;

			AbstractUniform* pUni;
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
			uint32 hash = FnvHash(fullName);
			assert(uniforms.find(hash) == uniforms.end());
			uniforms[hash] = pUni;
		}
	}
	return true;
}

void ShaderLoader::Destroy(ShaderData* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}