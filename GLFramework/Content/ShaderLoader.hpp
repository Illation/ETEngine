#pragma once
#include "../staticDependancies/glad/glad.h"
#include <string>

class ShaderLoader
{
public:
	ShaderLoader();
	~ShaderLoader();
	GLuint CreateShaderProgram(std::string vertexPath, std::string fragmentPath, GLuint& vertexShader, GLuint& fragmentShader);
	GLuint CreateGSProgram(std::string vertexPath, std::string geoPath, std::string fragmentPath
		, GLuint& vertexShader, GLuint& geoShader, GLuint& fragmentShader);
private:
	GLuint LoadShader(const char* filename, GLenum type);
};

