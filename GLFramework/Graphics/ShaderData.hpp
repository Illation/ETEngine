#pragma once
#include "../staticDependancies/glad/glad.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
class ShaderData
{
public:
	ShaderData(GLuint shaderProg, GLuint vertShader, GLuint fragShader);
	ShaderData(GLuint shaderProg, GLuint vertShader, GLuint geoShader, GLuint fragShader);
	~ShaderData();

	GLuint GetProgram() { return m_ShaderProgram; }

private:
	GLuint m_VertexShader;
	GLuint m_GeometryShader;
	GLuint m_FragmentShader;
	GLuint m_ShaderProgram;
	bool m_UseGS = false;
};

