#include "stdafx.h"
#include "Material.h"

#include "Shader.h"
#include "VertexInfo.h"

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>


Material::Material(std::string shaderFile) :
	m_ShaderFile(shaderFile)
{ }

void Material::Initialize()
{
	if (!m_IsInitialized)
	{
		//Load Shader
		m_Shader = ResourceManager::Instance()->GetAssetData<ShaderData>(GetHash(FileUtil::ExtractName(m_ShaderFile)));

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
		AccessShaderAttributes();

		m_IsInitialized = true;
	}
}

void Material::UploadVariables(mat4 matModel)
{
	Viewport::GetCurrentApiContext()->SetShader(m_Shader.get());
	//Upload matrices
	if (m_StandardTransform)
	{
		m_Shader->Upload("model"_hash, matModel);
	}

	UploadDerivedVariables();
}

void Material::UploadModelOnly(mat4 matModel)
{
	Viewport::GetCurrentApiContext()->SetShader(m_Shader.get());

	if (m_StandardTransform)
	{
		m_Shader->Upload("model"_hash, matModel);
	}
}

void Material::UploadNonInstanceVariables()
{
	UploadDerivedVariables();
}
