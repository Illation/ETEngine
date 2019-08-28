#include "stdafx.h"
#include "Material.h"

#include "Shader.h"
#include "MeshFilter.h"

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/ResourceManager.h>


Material::Material(std::string shaderFile) :
	m_ShaderFile(shaderFile)
{ }

void Material::Initialize()
{
	if (!m_IsInitialized)
	{
		//Load Shader
		m_Shader = ResourceManager::GetInstance()->GetAssetData<ShaderData>(GetHash(FileUtil::ExtractName(m_ShaderFile)));

		// determine layout flags and locations
		std::vector<ShaderData::T_AttributeLocation> const& attributes = m_Shader->GetAttributes();

		for (auto it = AttributeDescriptor::s_VertexAttributes.begin(); it != AttributeDescriptor::s_VertexAttributes.end(); ++it)
		{
			auto const attribIt = std::find_if(attributes.cbegin(), attributes.cend(), [it](ShaderData::T_AttributeLocation const& loc)
				{
					return it->second.name == loc.second.name;
				});

			if (attribIt != attributes.cend())
			{
				m_AttributeLocations.emplace_back(attribIt->first);
				m_LayoutFlags |= it->first;
			}
		}

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
		//m_Shader->Upload("model"_hash, matModel);
		//m_Shader->Upload("worldViewProj"_hash, CAMERA->GetViewProj());
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
		//m_Shader->Upload("model"_hash, matModel);
		//m_Shader->Upload("worldViewProj"_hash, matWVP);
		glUniformMatrix4fv(m_UniMatModel, 1, GL_FALSE, etm::valuePtr(matModel));
		glUniformMatrix4fv(m_UniMatWVP, 1, GL_FALSE, etm::valuePtr(matWVP));
	}

	UploadDerivedVariables();
}
