#include "stdafx.hpp"
#include "ShaderLoader.hpp"

#include "../Graphics/ShaderData.hpp"
#include "FileSystem/Entry.h"
#include <algorithm>
#include "FileSystem/FileUtil.h"
#include "Hash.h"
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
	cout << "Building Shader: " << assetFile << " . . . ";

	File* input = new File( assetFile, nullptr );
	if(!input->Open( FILE_ACCESS_MODE::Read ))
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    Opening shader file failed." << endl;
		return nullptr;
	}
	std::string shaderContent = FileUtil::AsText(input->Read());
	delete input; 
	input = nullptr;
	if(shaderContent.size() == 0)
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    Shader file is empty." << endl;
		return nullptr;
	}

	//Precompile
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
	GLuint vertexShader = CompileShader(vertSource, GL_VERTEX_SHADER);
	ParseUniforms(vertSource, uniforms);

	GLuint geoShader = 0;
	if (useGeo)
	{
		geoShader = CompileShader(geoSource, GL_GEOMETRY_SHADER);
		ParseUniforms(geoSource, uniforms);
	}

	GLuint fragmentShader = 0;
	if (useFrag)
	{
		fragmentShader = CompileShader(fragSource, GL_FRAGMENT_SHADER);
		ParseUniforms(fragSource, uniforms);
	}

	//Combine Shaders
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	if(useGeo)glAttachShader(shaderProgram, geoShader);
	if (useFrag)glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");

	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	if (useGeo)glDeleteShader(geoShader);
	if (useFrag)glDeleteShader(fragmentShader);

	GetUniformLocations(shaderProgram, uniforms);

	ShaderData* pShaderData = new ShaderData(shaderProgram);
	pShaderData->m_Name = assetFile;
	pShaderData->m_Uniforms = uniforms;
	
	cout << "  . . . SUCCESS!" << endl;
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
		cout << "  . . . FAILED!" << endl;
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
		cout << "    Compiling "<< sName << " shader failed." << endl;
		cout << "    " << buffer << endl;
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
			if (!(ReplaceInclude(extractedLine, assetFile)))
			{
				cout << "  . . . FAILED!" << endl;
				cout << "    Opening shader file failed." << endl; 
				return false;
			}
		}

		//Precompile types
		switch (state)
		{
		case INIT:
			if (extractedLine.find("<VERTEX>") != string::npos)
			{
				state = ParseState::VERT;
			}
			if (extractedLine.find("<GEOMETRY>") != string::npos)
			{
				useGeo = true;
				state = ParseState::GEO;
			}
			if (extractedLine.find("<FRAGMENT>") != string::npos)
			{
				state = ParseState::FRAG;
				useFrag = true;
			}
			break;
		case VERT:
			if (extractedLine.find("</VERTEX>") != string::npos)
			{
				state = ParseState::INIT;
				break;
			}
			vertSource += extractedLine;
			vertSource += "\n";
			break;
		case GEO:
			if (extractedLine.find("</GEOMETRY>") != string::npos)
			{
				state = ParseState::INIT;
				break;
			}
			geoSource += extractedLine;
			geoSource += "\n";
			break;
		case FRAG:
			if (extractedLine.find("</FRAGMENT>") != string::npos)
			{
				state = ParseState::INIT;
				break;
			}
			else if (extractedLine.find("#disable") != string::npos)
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
	uint32 lastS = static_cast<uint32>(max((int32)lastFS, (int32)lastBS));
	if(!(lastS == std::string::npos))basePath = assetFile.substr(0, lastS) + "/";

	uint32 firstQ = (uint32)line.find("\"");
	uint32 lastQ = (uint32)line.rfind("\"");
	if ((firstQ == std::string::npos) ||
		(lastQ == std::string::npos) ||
		lastQ <= firstQ)
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    invalid include syntax." << endl;
		cout << line << endl;
		return false;
	}
	firstQ++;
	std::string path = basePath + line.substr(firstQ, lastQ - firstQ);

	File* input = new File( path, nullptr );
	if(!input->Open( FILE_ACCESS_MODE::Read ))
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    Opening shader file failed." << endl;
		return false;
	}
	std::string shaderContent = FileUtil::AsText(input->Read());
	delete input;
	input = nullptr;
	if(shaderContent.size() == 0)
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    Shader file is empty." << endl;
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
				cout << "  . . . FAILED!" << endl;
				cout << "    Opening shader file failed." << endl; 
				return false;
			}
		}
		
		line += extractedLine + "\n";
	}
	return true;
}

void ShaderLoader::ParseUniforms(const std::string &source, std::map<uint32, AbstractUniform*> &uniforms)
{
	size_t readPos = 0;
	while (readPos < source.size())
	{
		readPos = source.find("uniform", readPos);
		if (readPos != std::string::npos)
		{
			NextCharNextWS(source, readPos);

			std::string typeName = ReadUniformName(source, readPos);

			std::vector<std::string> uniNames;
			do uniNames.push_back(ReadUniformName(source, readPos));
			while (source[readPos++] == ',');

			for (auto uniName : uniNames)
			{
				if (typeName == "bool") uniforms[FnvHash(uniName)] = new Uniform<bool>();
				else if (typeName == "int") uniforms[FnvHash(uniName)] = new Uniform<int32>();
				else if (typeName == "float") uniforms[FnvHash(uniName)] = new Uniform<float>();
				else if (typeName == "vec2") uniforms[FnvHash(uniName)] = new Uniform<vec2>();
				else if (typeName == "vec3") uniforms[FnvHash(uniName)] = new Uniform<vec3>();
				else if (typeName == "vec4") uniforms[FnvHash(uniName)] = new Uniform<vec4>();
				else if (typeName == "mat3") uniforms[FnvHash(uniName)] = new Uniform<mat3>();
				else if (typeName == "mat4") uniforms[FnvHash(uniName)] = new Uniform<mat4>();
				else
				{
					std::cout << "unrecognized uniform typename: " << uniName << std::endl;
					return;
				}
			}

			continue;
		}
		readPos = source.size();
	}
}

void ShaderLoader::NextCharNextWS(const std::string &source, size_t &readPos)
{
	while (readPos < source.size() && !std::isspace(source[readPos]))readPos++;
	while (readPos < source.size() && std::isspace(source[readPos]))readPos++;
}

std::string ShaderLoader::ReadUniformName(const std::string &source, size_t &readPos)
{
	std::string ret;
	size_t origin = readPos;
	while (readPos < source.size()
		&& !std::isspace(source[readPos])
		&& !(source[readPos] == ';')
		&& !(source[readPos] == ',')
		&& !(source[readPos] == '=')
	)
		readPos++;
	ret = source.substr(origin, readPos - origin);
	if (std::isspace(source[readPos]))NextCharNextWS(source, --readPos);
	if (source[readPos] == '=')
	{
		readPos = min(source.find(",", readPos), source.find(";", readPos));
	}
	if (source[readPos] == ',')
	{
		if (std::isspace(source[++readPos]))NextCharNextWS(source, --readPos);
	}
	return ret;
}

void ShaderLoader::GetUniformLocations(GLuint shaderProgram, std::map<uint32, AbstractUniform*> &uniforms)
{

}

void ShaderLoader::Destroy(ShaderData* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}