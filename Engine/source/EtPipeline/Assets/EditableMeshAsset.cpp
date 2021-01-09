#include "stdafx.h"
#include "EditableMeshAsset.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>  
#include <assimp/postprocess.h>

#include <ext-mikktspace/mikktspace.h>

#include <EtBuild/EngineVersion.h>

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/IO/BinaryWriter.h>

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

//-------------------------------------
// EditableMeshAsset::GenerateInternal
//
bool EditableMeshAsset::GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath)
{
	ET_ASSERT(m_RuntimeAssets.size() == 1u);
	m_RuntimeAssets[0].m_HasGeneratedData = true; 

	// load intermediate data format
	//-------------------------------
	std::string const extension = core::FileUtil::ExtractExtension(m_Asset->GetName());
	render::MeshDataContainer* meshContainer = nullptr;
	if (extension == "gltf")
	{
		meshContainer = LoadGLTF(m_Asset->GetLoadData(), dbPath + m_Asset->GetPath(), extension);
	}
	else
	{
		meshContainer = LoadAssimp(m_Asset->GetLoadData(), extension);
	}

	if (meshContainer == nullptr)
	{
		ET_ASSERT(false, "Failed to load mesh container");
		return false;
	}

	// fetch info so we can calculate file size
	//-------------------------------------------
	uint64 const indexCount = static_cast<uint64>(meshContainer->m_Indices.size());
	uint64 const vertexCount = static_cast<uint64>(meshContainer->m_VertexCount);

	// #todo: might be okay to store index buffer with 16bits per index
	render::E_DataType const indexDataType = render::E_DataType::UInt;
	render::T_VertexFlags const flags = meshContainer->GetFlags();
	math::Sphere const boundingSphere = meshContainer->GetBoundingSphere();

	size_t const iBufferSize = indexCount * static_cast<size_t>(render::DataTypeInfo::GetTypeSize(indexDataType));
	size_t const vBufferSize = vertexCount * static_cast<size_t>(render::AttributeDescriptor::GetVertexSize(flags));

	// init binary writer
	//--------------------
	core::BinaryWriter binWriter(m_RuntimeAssets[0].m_GeneratedData);
	binWriter.FormatBuffer(render::MeshAsset::s_Header.size() +
		build::Version::s_Name.size() + 1u + 
		sizeof(uint64) + // index count
		sizeof(uint64) + // vertex count
		sizeof(render::E_DataType) +
		sizeof(render::T_VertexFlags) +
		sizeof(float) * 4u + // bounding sphere - pos (3) + radius (1)
		iBufferSize +
		vBufferSize);

	// write header
	//--------------
	binWriter.WriteString(render::MeshAsset::s_Header);
	binWriter.WriteNullString(build::Version::s_Name);

	binWriter.Write(indexCount);
	binWriter.Write(vertexCount);

	binWriter.Write(indexDataType);
	binWriter.Write(flags);
	binWriter.WriteVector(boundingSphere.pos);
	binWriter.Write(boundingSphere.radius);

	// writer indices
	//----------------
	// we just assume index data type will be the same as what is in the mesh container for now.. in the future we might have to convert
	binWriter.WriteData(reinterpret_cast<uint8 const*>(meshContainer->m_Indices.data()), iBufferSize);

	// write vertices
	//----------------
	for (size_t vertIdx = 0u; vertIdx < static_cast<size_t>(vertexCount); vertIdx++)
	{
		if (flags & render::E_VertexFlag::POSITION)
		{
			binWriter.WriteVector(meshContainer->m_Positions[vertIdx]);
		}

		if (flags & render::E_VertexFlag::NORMAL)
		{
			binWriter.WriteVector(meshContainer->m_Normals[vertIdx]);
		}

		if (flags & render::E_VertexFlag::BINORMAL)
		{
			binWriter.WriteVector(meshContainer->m_BiNormals[vertIdx]);
		}

		if (flags & render::E_VertexFlag::TANGENT)
		{
			binWriter.WriteVector(meshContainer->m_Tangents[vertIdx]);
		}

		if (flags & render::E_VertexFlag::COLOR)
		{
			binWriter.WriteVector(meshContainer->m_Colors[vertIdx]);
		}

		if (flags & render::E_VertexFlag::TEXCOORD)
		{
			binWriter.WriteVector(meshContainer->m_TexCoords[vertIdx]);
		}
	}

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
