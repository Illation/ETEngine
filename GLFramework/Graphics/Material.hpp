#pragma once
#include "../StaticDependancies/glad/glad.h"
#include <glm\glm.hpp>
#include <string>

class Material
{
public:
	Material(std::string VSfile, std::string FSfile, 
		std::string GSfile = std::string(""), 
		bool UseGeometryShader = false);
	virtual ~Material();

	virtual void Initialize();
	virtual void SpecifyInputLayout() = 0;
	void UploadVariables(glm::mat4 matModel);

protected:
	virtual void LoadTextures() = 0;
	virtual void AccessShaderAttributes() = 0;

	virtual void UploadDerivedVariables() = 0;

protected:
	GLuint m_ShaderProgram;

	GLint m_UniMatModel;
	GLint m_UniMatWVP;
	bool m_UseGS=false;
private:
	GLuint m_VertexShader;
	GLuint m_GeometryShader;
	GLuint m_FragmentShader;

	std::string m_VSfile, m_GSfile, m_FSfile;
};

