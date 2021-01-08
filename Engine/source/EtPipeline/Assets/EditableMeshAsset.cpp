#include "stdafx.h"
#include "EditableMeshAsset.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>  
#include <assimp/postprocess.h>

#include <ext-mikktspace/mikktspace.h>

#include <EtCore/FileSystem/FileUtil.h>

#include <EtPipeline/Import/GLTF.h>


namespace et {
namespace pl {


//=====================
// Editable Mesh Asset
//=====================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableMeshAsset, "editable mesh asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableMeshAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableMeshAsset) // force the asset class to be linked as it is only used in reflection


//-----------------------------------
// EditableMeshAsset::LoadFromMemory
//
// temporary - in the future we will create optimized data with the mesh importer instead
//
bool EditableMeshAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	std::string const extension = core::FileUtil::ExtractExtension(m_Asset->GetName());
	render::MeshDataContainer* meshContainer = nullptr;

	meshContainer = LoadAssimp(data, extension);
	if (meshContainer == nullptr)
	{
		LOG("MeshAsset::LoadFromMemory > Failed to load mesh asset!", core::LogLevel::Warning);
		return false;
	}

	if (meshContainer->m_Name.empty())
	{
		meshContainer->m_Name = m_Asset->GetName();
	}

	SetData(new render::MeshData(meshContainer));

	delete meshContainer;
	return true;
}

//---------------------------------
// EditableMeshAsset::LoadAssimp
//
// Convert assimp mesh to a CPU side MeshDataContainer
//
render::MeshDataContainer* EditableMeshAsset::LoadAssimp(std::vector<uint8> const& data, std::string const& extension)
{
	// load the mesh data into an assimp scene and do all necessary conversions
	//--------------------------------------------------------------------------

	Assimp::Importer assimpImporter;

	uint32 importFlags =
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenSmoothNormals |
		aiProcess_PreTransformVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_FlipUVs |
		aiProcess_OptimizeMeshes |
		aiProcess_MakeLeftHanded;

	aiScene const* const assimpScene = assimpImporter.ReadFileFromMemory(data.data(), data.size(), importFlags, extension.c_str());
	if (assimpScene == nullptr)
	{
		LOG(FS("Loading scene with assimp failed: %s", assimpImporter.GetErrorString()), core::LogLevel::Warning);
		return nullptr;
	}

	if (!(assimpScene->HasMeshes()))
	{
		LOG("Assimp scene found empty!", core::LogLevel::Warning);
		return nullptr;
	}

	aiMesh const* const assimpMesh = assimpScene->mMeshes[0];
	if (!(assimpMesh->HasPositions()))
	{
		LOG("Assimp mesh found empty!", core::LogLevel::Warning);
		return nullptr;
	}


	// start with minumum mesh data
	//------------------------------

	render::MeshDataContainer* meshData = new render::MeshDataContainer();
	meshData->m_Name = assimpMesh->mName.C_Str();
	meshData->m_VertexCount = assimpMesh->mNumVertices;

	// indices
	if (assimpMesh->mNumFaces > 0)
	{
		for (size_t i = 0; i < assimpMesh->mNumFaces; i++)
		{
			meshData->m_Indices.push_back(static_cast<uint32>(assimpMesh->mFaces[i].mIndices[0]));
			meshData->m_Indices.push_back(static_cast<uint32>(assimpMesh->mFaces[i].mIndices[1]));
			meshData->m_Indices.push_back(static_cast<uint32>(assimpMesh->mFaces[i].mIndices[2]));
		}
	}
	else
	{
		LOG("MeshAsset::LoadAssimp > No indices found!", core::LogLevel::Warning);
	}

	// positions
	if (assimpMesh->mNumVertices > 0)
	{
		for (size_t i = 0; i < assimpMesh->mNumVertices; i++)
		{
			meshData->m_Positions.push_back(vec3(
				assimpMesh->mVertices[i].x,
				assimpMesh->mVertices[i].y,
				assimpMesh->mVertices[i].z));
		}
	}

	// everything else is optional
	//-----------------------------

	// normals
	if (assimpMesh->HasNormals())
	{
		for (size_t i = 0; i < assimpMesh->mNumVertices; i++)
		{
			meshData->m_Normals.push_back(vec3(
				assimpMesh->mNormals[i].x,
				assimpMesh->mNormals[i].y,
				assimpMesh->mNormals[i].z));
		}
	}

	// tangent space
	if (assimpMesh->HasTangentsAndBitangents())
	{
		for (size_t i = 0; i < assimpMesh->mNumVertices; i++)
		{
			meshData->m_Tangents.push_back(vec3(
				assimpMesh->mTangents[i].x,
				assimpMesh->mTangents[i].y,
				assimpMesh->mTangents[i].z));
			meshData->m_BiNormals.push_back(vec3(
				assimpMesh->mBitangents[i].x,
				assimpMesh->mBitangents[i].y,
				assimpMesh->mBitangents[i].z));
		}
	}

	// vertex colors
	if (assimpMesh->HasVertexColors(0))
	{
		for (size_t i = 0; i < assimpMesh->mNumVertices; i++)
		{
			meshData->m_Colors.push_back(vec4(
				assimpMesh->mColors[0][i].r,
				assimpMesh->mColors[0][i].g,
				assimpMesh->mColors[0][i].b,
				assimpMesh->mColors[0][i].a));
		}
	}

	// tex coords
	if (assimpMesh->HasTextureCoords(0))
	{
		if (!(assimpMesh->mNumUVComponents[0] == 2))
		{
			LOG("UV dimensions don't match internal layout!", core::LogLevel::Warning);
		}

		for (size_t i = 0; i < assimpMesh->mNumVertices; i++)
		{
			meshData->m_TexCoords.push_back(vec2(
				assimpMesh->mTextureCoords[0][i].x,
				assimpMesh->mTextureCoords[0][i].y));
		}
	}

	return meshData;
}

//---------------------------------
// EditableMeshAsset::LoadGLTF
//
// Convert a gltf asset to a CPU side MeshDataContainer
//
render::MeshDataContainer* EditableMeshAsset::LoadGLTF(std::vector<uint8> const& data, std::string const& path, std::string const& extension)
{
	glTF::glTFAsset asset;
	if (!glTF::ParseGLTFData(data, path, extension, asset))
	{
		LOG("failed to load the glTF asset", core::LogLevel::Warning);
		return nullptr;
	}

	std::vector<render::MeshDataContainer*> containers;
	if (!glTF::GetMeshContainers(asset, containers))
	{
		LOG("failed to construct mesh data containers from glTF", core::LogLevel::Warning);
		return nullptr;
	}

	if (containers.size() == 0)
	{
		LOG("no mesh data containers found in glTF asset", core::LogLevel::Warning);
		return nullptr;
	}

	render::MeshDataContainer* ret = containers[0];

	if (containers.size() > 1)
	{
		for (size_t i = 1u; i < containers.size(); ++i)
		{
			delete containers[i];
		}
	}

	return ret;
}


} // namespace pl
} // namespace et
