#pragma once
#include "../StaticDependancies/glad/glad.h"
#include <glm\glm.hpp>
#include <string>

class ShaderData;

class Material
{
public:
	Material(std::string shaderFile);
	virtual ~Material();

	virtual void Initialize();
	virtual void SpecifyInputLayout() = 0;
	void UploadVariables(glm::mat4 matModel);

protected:
	virtual void LoadTextures() = 0;
	virtual void AccessShaderAttributes() = 0;

	virtual void UploadDerivedVariables() = 0;

protected:
	ShaderData* m_Shader;
	GLint m_UniMatModel;
	GLint m_UniMatWVP;
private:

	std::string m_ShaderFile;
};

