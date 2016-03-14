#include "ShaderLoader.hpp"

#include <iostream>
#include <fstream>

#include "../Graphics/ShaderData.hpp"

ShaderLoader::ShaderLoader()
{
}


ShaderLoader::~ShaderLoader()
{
}

ShaderData* ShaderLoader::LoadContent(const std::string& assetFile)
{
	using namespace std;

	cout << "Compiling Shader: " << assetFile << " . . . ";

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
	if(useGeo)geoShader = CompileShader(fragSource, GL_GEOMETRY_SHADER);
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
	return new ShaderData(shaderProgram, vertexShader, fragmentShader);
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
		cout << "    Compiling shader failed." << endl;
	}

	return shader;
}

void ShaderLoader::Destroy(ShaderData* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}