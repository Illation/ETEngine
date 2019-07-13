#pragma once
#include <string>

class ShaderData;

class Material
{
public:
	Material(std::string shaderFile);
	virtual ~Material();

	virtual void Initialize();
	void SpecifyInputLayout();
	uint32 GetLayoutFlags() { return m_LayoutFlags; }
	void UploadVariables(mat4 matModel);
	void UploadVariables(mat4 matModel, const mat4 &matWVP);
	bool IsForwardRendered(){ return m_DrawForward; }

protected:
	virtual void LoadTextures() = 0;
	virtual void AccessShaderAttributes() = 0;

	virtual void UploadDerivedVariables() = 0;

protected:
	uint32 m_LayoutFlags = 0;
	bool m_DrawForward = false;
	bool m_StandardTransform = true;
	ShaderData* m_Shader;
	GLint m_UniMatModel;
	GLint m_UniMatWVP;
private:

	bool m_IsInitialized = false;
	std::string m_ShaderFile;
};

