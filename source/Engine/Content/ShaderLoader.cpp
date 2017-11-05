#include "stdafx.hpp"
#include "ShaderLoader.hpp"

#include "../Graphics/ShaderData.hpp"
#include "FileSystem/Entry.h"
#include <algorithm>
#include "FileSystem/FileUtil.h"

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

	//Compile
	GLuint vertexShader = CompileShader(vertSource, GL_VERTEX_SHADER);
	GLuint geoShader = 0;
	if(useGeo)geoShader = CompileShader(geoSource, GL_GEOMETRY_SHADER);
	GLuint fragmentShader = 0;
	if (useFrag) fragmentShader = CompileShader(fragSource, GL_FRAGMENT_SHADER);

	//Combine Shaders
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	if(useGeo)glAttachShader(shaderProgram, geoShader);
	if (useFrag)glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);

	cout << "  . . . SUCCESS!" << endl;

	ShaderData* pShaderData = nullptr;
	if (useGeo) pShaderData = new ShaderData(shaderProgram, vertexShader, geoShader, fragmentShader);
	else if (useFrag) pShaderData =  new ShaderData(shaderProgram, vertexShader, fragmentShader);
	else pShaderData = new ShaderData(shaderProgram, vertexShader);
	pShaderData->m_Name = assetFile;
	
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
	uint32 lastFS = assetFile.rfind("/");
	uint32 lastBS = assetFile.rfind("\\");
	uint32 lastS = static_cast<uint32>(max((int32)lastFS, (int32)lastBS));
	if(!(lastS == std::string::npos))basePath = assetFile.substr(0, lastS) + "/";

	uint32 firstQ = line.find("\"");
	uint32 lastQ = line.rfind("\"");
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

void ShaderLoader::Destroy(ShaderData* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}