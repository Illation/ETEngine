#include "stdafx.hpp"
#include "ShaderLoader.hpp"

#include "../Graphics/ShaderData.hpp"
#include <algorithm>

ShaderLoader::ShaderLoader()
{
}


ShaderLoader::~ShaderLoader()
{
}

ShaderData* ShaderLoader::LoadContent(const std::string& assetFile)
{
	using namespace std;

	cout << "Building Shader: " << assetFile << " . . . ";

	string vertSource;
	string geoSource;
	string fragSource;

	enum ParseState {
		INIT,
		VERT,
		GEO,
		FRAG
	} state = ParseState::INIT;
	bool useGeo = false;

	string extractedLine;
	ifstream shaderFile;
	shaderFile.open(assetFile);
	if (!shaderFile)
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    Opening shader file failed." << endl;
		return nullptr;
	}
	while (shaderFile.eof() == false)
	{
		//Get the line
		getline(shaderFile, extractedLine);

		//Includes
		if (extractedLine.find("#include") != std::string::npos)
		{
			if (!(ReplaceInclude(extractedLine, assetFile)))
			{
				cout << "  . . . FAILED!" << endl;
				cout << "    Opening shader file failed." << endl; 
				return nullptr;
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
			fragSource += extractedLine;
			fragSource += "\n";
			break;
		}
	}
	shaderFile.close();

	//Compile
	GLuint vertexShader = CompileShader(vertSource, GL_VERTEX_SHADER);
	GLuint geoShader;
	if(useGeo)geoShader = CompileShader(geoSource, GL_GEOMETRY_SHADER);
	GLuint fragmentShader = CompileShader(fragSource, GL_FRAGMENT_SHADER);

	//Combine Shaders
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	if(useGeo)glAttachShader(shaderProgram, geoShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);

	cout << "  . . . SUCCESS!" << endl;
	if (useGeo)return new ShaderData(shaderProgram, vertexShader, geoShader, fragmentShader);
	ShaderData* pShaderData =  new ShaderData(shaderProgram, vertexShader, fragmentShader);
	pShaderData->m_Name = assetFile;
	return pShaderData;
}

GLuint ShaderLoader::CompileShader(const std::string &shaderSourceStr, GLenum type)
{
	using namespace std;
	const char *shaderSource = shaderSourceStr.c_str();
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &shaderSource, NULL);

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

bool ShaderLoader::ReplaceInclude(std::string &line, const std::string &assetFile)
{
	std::string basePath = "";
	size_t lastFS = assetFile.rfind("/");
	size_t lastBS = assetFile.rfind("\\");
	size_t lastS = (size_t)std::max((int32)lastFS, (int32)lastBS);
	if(!(lastS == std::string::npos))basePath = assetFile.substr(0, lastS) + "/";

	size_t firstQ = line.find("\"");
	size_t lastQ = line.rfind("\"");
	if ((firstQ == std::string::npos) ||
		(lastQ == std::string::npos) ||
		lastQ <= firstQ)
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    invalid include syntax." << endl;
		cout << line << endl;
		return nullptr;
	}
	firstQ++;
	std::string path = basePath + line.substr(firstQ, lastQ - firstQ);

	ifstream shaderFile;
	shaderFile.open(path);
	if (!shaderFile)
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    Opening shader file \"" << path << "\" failed." << endl;
		return nullptr;
	}
	std::string ret;
	std::string extractedLine;
	while (shaderFile.eof() == false)
	{
		//Get the line
		getline(shaderFile, extractedLine);

		//Includes
		if (extractedLine.find("#include") != std::string::npos)
		{
			if (!(ReplaceInclude(extractedLine, assetFile)))
			{
				cout << "  . . . FAILED!" << endl;
				cout << "    Opening shader file failed." << endl; 
				return nullptr;
			}
		}
		
		ret += extractedLine;
		ret += "\n";
	}
	shaderFile.close();
	line = ret;
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