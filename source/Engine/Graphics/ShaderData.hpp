#pragma once
#include "../staticDependancies/glad/glad.h"
//Use lefthanded coordinate system
#ifndef GLM_LEFT_HANDED
	#define GLM_LEFT_HANDED
#endif
#include <glm.hpp>
#include <string>
#include <vector>
class ShaderData
{
public:
	ShaderData(GLuint shaderProg, GLuint vertShader, GLuint geoShader, GLuint fragShader);
	ShaderData(GLuint shaderProg, GLuint vertShader, GLuint fragShader);
	ShaderData(GLuint shaderProg, GLuint vertShader);
	~ShaderData();

	GLuint GetProgram() { return m_ShaderProgram; }

	string GetName() { return m_Name; }
private:
	friend class ShaderLoader;

	GLuint m_VertexShader;
	GLuint m_GeometryShader;
	GLuint m_FragmentShader;
	GLuint m_ShaderProgram;
	bool m_UseGS = false;
	bool m_UseFS = false;

	string m_Name;
};

