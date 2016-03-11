#include "ShaderLoader.hpp"

#include <iostream>
#include <fstream>



ShaderLoader::ShaderLoader()
{
}


ShaderLoader::~ShaderLoader()
{
}

GLuint ShaderLoader::LoadShader(const char* filename, GLenum type)
{
	using namespace std;

	cout << "Compiling Shader: " << filename << " . . . ";

	string shaderSourceStr;

	string extractedLine;
	ifstream shaderFile;
	shaderFile.open(filename);
	if (!shaderFile)
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    Opening shader file failed." << endl;
		return -1;
	}
	while (shaderFile.eof() == false)
	{
		getline(shaderFile, extractedLine);
		shaderSourceStr += extractedLine;
		shaderSourceStr += "\n";
	}
	shaderFile.close();

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
	else
	{
		cout << "  . . . SUCCESS!" << endl;
	}

	return shader;
}

GLuint ShaderLoader::CreateShaderProgram(std::string vertexPath, std::string fragmentPath, GLuint& vertexShader, GLuint& fragmentShader)
{
	vertexShader = LoadShader(vertexPath.c_str(), GL_VERTEX_SHADER);
	fragmentShader = LoadShader(fragmentPath.c_str(), GL_FRAGMENT_SHADER);
	//Combine Shaders
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	return shaderProgram;
}

GLuint ShaderLoader::CreateGSProgram(std::string vertexPath, std::string geoPath, std::string fragmentPath
	, GLuint& vertexShader, GLuint& geoShader, GLuint& fragmentShader)
{
	vertexShader = LoadShader(vertexPath.c_str(), GL_VERTEX_SHADER);
	geoShader = LoadShader(vertexPath.c_str(), GL_GEOMETRY_SHADER);
	fragmentShader = LoadShader(fragmentPath.c_str(), GL_FRAGMENT_SHADER);
	//Combine Shaders
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, geoShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	return shaderProgram;
}
