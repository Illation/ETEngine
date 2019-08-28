#include "stdafx.h"
#include "Material.h"

#include "Shader.h"
#include "MeshFilter.h"

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/ResourceManager.h>


Material::Material(std::string shaderFile) :
	m_ShaderFile(shaderFile)
{
}
Material::~Material()
{
}

void Material::Initialize()
{
	if (!m_IsInitialized)
	{
		//Load Shader
		m_Shader = ResourceManager::GetInstance()->GetAssetData<ShaderData>(GetHash(FileUtil::ExtractName(m_ShaderFile)));

		//Jup (maybe temporary with texture manager)
		LoadTextures();

		//Get Access to uniforms
		if(m_StandardTransform)
		{
			m_UniMatModel = glGetUniformLocation(m_Shader->GetProgram(), "model");
			m_UniMatWVP = glGetUniformLocation(m_Shader->GetProgram(), "worldViewProj");
		}
		AccessShaderAttributes();

		m_IsInitialized = true;
	}
}

void Material::UploadVariables(mat4 matModel)
{
	STATE->SetShader(m_Shader.get());
	//Upload matrices
	if (m_StandardTransform)
	{
		glUniformMatrix4fv(m_UniMatModel, 1, GL_FALSE, etm::valuePtr(matModel));
		glUniformMatrix4fv(m_UniMatWVP, 1, GL_FALSE, etm::valuePtr(CAMERA->GetViewProj()));
	}

	UploadDerivedVariables();
}
void Material::UploadVariables(mat4 matModel, const mat4 &matWVP)
{
	STATE->SetShader(m_Shader.get());
	//Upload matrices
	if (m_StandardTransform)
	{
		glUniformMatrix4fv(m_UniMatModel, 1, GL_FALSE, etm::valuePtr(matModel));
		glUniformMatrix4fv(m_UniMatWVP, 1, GL_FALSE, etm::valuePtr(matWVP));
	}

	UploadDerivedVariables();
}

bool Material::GetAttributeLocations(std::vector<int32>& locations) const
{
	for (auto it = AttributeDescriptor::s_VertexAttributes.begin(); it != AttributeDescriptor::s_VertexAttributes.end(); ++it)
	{
		if (m_LayoutFlags & it->first)
		{
			char const* attribName = it->second.name.c_str();
			GLint attrib = glGetAttribLocation(m_Shader->GetProgram(), attribName);

			if (attrib >= 0)
			{
				locations.emplace_back(attrib);
			}
			else
			{
				LOG(FS("Could not find attribute '%s' in shader '%s'!", attribName, m_Shader->GetName().c_str()), LogLevel::Error);
				return false;
			}
		}
	}

	return true;
}
