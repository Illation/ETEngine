#pragma once
#include "../StaticDependancies/glad/glad.h"
//Use lefthanded coordinate system
#ifndef GLM_LEFT_HANDED
	#define GLM_LEFT_HANDED
#endif
#include <glm\glm.hpp>
#include <string>

class ShaderData;

class Material
{
public:
	Material(std::string shaderFile);
	virtual ~Material();

	virtual void Initialize();
	void SpecifyInputLayout();
	unsigned GetLayoutFlags() { return m_LayoutFlags; }
	void UploadVariables(glm::mat4 matModel);
	void UploadVariables(glm::mat4 matModel, const glm::mat4 &matWVP);
	bool IsForwardRendered(){ return m_DrawForward; }

protected:
	virtual void LoadTextures() = 0;
	virtual void AccessShaderAttributes() = 0;

	virtual void UploadDerivedVariables() = 0;

protected:
	unsigned m_LayoutFlags = 0;
	bool m_DrawForward = false;
	ShaderData* m_Shader;
	GLint m_UniMatModel;
	GLint m_UniMatWVP;
private:

	bool m_IsInitialized = false;
	std::string m_ShaderFile;
};

