#pragma once
#include "VertexInfo.h"

#include <string>

#include <EtCore/Content/AssetPointer.h>

class ShaderData;

class Material
{
public:
	Material(std::string shaderFile);
	virtual ~Material() = default;

	virtual void Initialize();
	void UploadVariables(mat4 matModel);
	void UploadVariables(mat4 matModel, const mat4 &matWVP);

	T_VertexFlags GetLayoutFlags() const { return m_LayoutFlags; }
	std::vector<int32> const& GetAttributeLocations() const { return m_AttributeLocations; }
	bool IsForwardRendered() const { return m_DrawForward; }

protected:
	virtual void LoadTextures() = 0;
	virtual void AccessShaderAttributes() = 0;
	virtual void UploadDerivedVariables() = 0;

protected:
	bool m_DrawForward = false;
	bool m_StandardTransform = true;

	AssetPtr<ShaderData> m_Shader;

	GLint m_UniMatModel;
	GLint m_UniMatWVP;

private:
	T_VertexFlags m_LayoutFlags = 0u;
	std::vector<int32> m_AttributeLocations;

	bool m_IsInitialized = false;
	std::string m_ShaderFile;
};

