#include "stdafx.hpp"
#include "MeshFilterLoader.hpp"

#include "../Graphics/MeshFilter.hpp"
#include "../FileSystem/Entry.h"

#include <Importer.hpp>
#include <scene.h>  
#include <postprocess.h>
#include "GLTF.h"

#define ASSIMP_BUILD_BOOST_WORAROUND

MeshFilterLoader::MeshFilterLoader()
{
}


MeshFilterLoader::~MeshFilterLoader()
{
}

MeshFilter* MeshFilterLoader::LoadContent(const std::string& assetFile)
{
	logPos = Logger::GetCursorPosition();
	loadingString = std::string("Loading Mesh: ") + assetFile + " . . .";

	LOG(loadingString + " . . . opening file          ", Info, false, logPos);

	File* input = new File(assetFile, nullptr);
	if (!input->Open(FILE_ACCESS_MODE::Read))
	{
		LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
		LOG("    Opening font file failed.", Warning);
		return nullptr;
	}
	std::vector<uint8> binaryContent = input->Read();
	std::string extension = input->GetExtension();
	std::string filename = input->GetName();
	delete input;
	input = nullptr;
	if (binaryContent.size() == 0)
	{
		LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
		LOG("    Font file is empty.", Warning);
		return nullptr;
	}

	MeshFilter* pMesh = nullptr;

	if ((extension == "gltf") || (extension == "glb"))
	{
		LOG(loadingString + " . . . loading gltf          ", Info, false, logPos);
		pMesh = LoadGLTF(binaryContent, extension);
	}
	else
	{
		LOG(loadingString + " . . . loading assimp          ", Info, false, logPos);
		pMesh = LoadAssimp(binaryContent, extension);
	}

	if (!pMesh)
	{
		LOG(loadingString + " . . . FAILED!         ", Warning, false, logPos);
		return nullptr;
	}

	if (pMesh->m_Name == "") pMesh->m_Name = filename;

	LOG(loadingString + " . . . SUCCESS!          ", Info, false, logPos);
	return pMesh;
}

void MeshFilterLoader::Destroy(MeshFilter* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}

MeshFilter* MeshFilterLoader::LoadAssimp(const std::vector<uint8>& binaryContent, const std::string &ext)
{
	//Get the assimp mesh
	auto pImporter = new Assimp::Importer();

	uint32 importFlags = 
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenSmoothNormals |
		aiProcess_PreTransformVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_FlipUVs |
		aiProcess_OptimizeMeshes |
		aiProcess_OptimizeGraph |
		aiProcess_MakeLeftHanded;

	const aiScene* pAssimpScene = pImporter->ReadFileFromMemory(binaryContent.data(), binaryContent.size(), importFlags, ext.c_str());
	if (!pAssimpScene)
	{
		LOG("	loading scene with assimp failed: ", Warning);
		LOG(pImporter->GetErrorString(), Warning);
		return nullptr;
	}
	if (!(pAssimpScene->HasMeshes()))
	{
		LOG("	scene found empty", Warning);
		return nullptr;
	}
	const aiMesh *pAssimpMesh = pAssimpScene->mMeshes[0];
	if (!(pAssimpMesh->HasPositions()))
	{
		LOG("	mesh found empty", Warning);
		return nullptr;
	}

	MeshFilter* pMesh = new MeshFilter();
	pMesh->m_Name = pAssimpMesh->mName.C_Str();
	pMesh->m_VertexCount = pAssimpMesh->mNumVertices;
	pMesh->m_IndexCount = pAssimpMesh->mNumFaces * 3;

	if (pAssimpMesh->mNumVertices > 0)
	{
		LOG(loadingString + " . . . adding positions          ", Info, false, logPos);
		pMesh->m_SupportedFlags |= VertexFlags::POSITION;
		for (size_t i = 0; i < pAssimpMesh->mNumVertices; i++)
		{
			pMesh->m_Positions.push_back(vec3(
				pAssimpMesh->mVertices[i].x,
				pAssimpMesh->mVertices[i].y,
				pAssimpMesh->mVertices[i].z));
		}
	}
	if (pAssimpMesh->mNumFaces > 0)
	{
		LOG(loadingString + " . . . adding indices          ", Info, false, logPos);
		for (size_t i = 0; i < pAssimpMesh->mNumFaces; i++)
		{
			pMesh->m_Indices.push_back((GLuint)(pAssimpMesh->mFaces[i].mIndices[0]));
			pMesh->m_Indices.push_back((GLuint)(pAssimpMesh->mFaces[i].mIndices[1]));
			pMesh->m_Indices.push_back((GLuint)(pAssimpMesh->mFaces[i].mIndices[2]));
		}
	}
	else LOG("No indices found!", Warning);
	if (pAssimpMesh->HasNormals())
	{
		LOG(loadingString + " . . . adding normals          ", Info, false, logPos);
		pMesh->m_SupportedFlags |= VertexFlags::NORMAL;
		for (size_t i = 0; i < pAssimpMesh->mNumVertices; i++)
		{
			pMesh->m_Normals.push_back(vec3(
				pAssimpMesh->mNormals[i].x,
				pAssimpMesh->mNormals[i].y,
				pAssimpMesh->mNormals[i].z));
		}
	}
	if (pAssimpMesh->HasTangentsAndBitangents())
	{
		LOG(loadingString + " . . . adding (bi)tangents          ", Info, false, logPos);
		pMesh->m_SupportedFlags |= VertexFlags::TANGENT;
		pMesh->m_SupportedFlags |= VertexFlags::BINORMAL;
		for (size_t i = 0; i < pAssimpMesh->mNumVertices; i++)
		{
			pMesh->m_Tangents.push_back(vec3(
				pAssimpMesh->mTangents[i].x,
				pAssimpMesh->mTangents[i].y,
				pAssimpMesh->mTangents[i].z));
			pMesh->m_BiNormals.push_back(vec3(
				pAssimpMesh->mBitangents[i].x,
				pAssimpMesh->mBitangents[i].y,
				pAssimpMesh->mBitangents[i].z));
		}
	}
	if (pAssimpMesh->HasVertexColors(0))
	{
		LOG(loadingString + " . . . adding vertex colors          ", Info, false, logPos);
		pMesh->m_SupportedFlags |= VertexFlags::COLOR;
		for (size_t i = 0; i < pAssimpMesh->mNumVertices; i++)
		{
			pMesh->m_Colors.push_back(vec4(
				pAssimpMesh->mColors[0][i].r,
				pAssimpMesh->mColors[0][i].g,
				pAssimpMesh->mColors[0][i].b,
				pAssimpMesh->mColors[0][i].a));
		}
	}
	if (pAssimpMesh->HasTextureCoords(0))
	{
		LOG(loadingString + " . . . adding texCoords          ", Info, false, logPos);
		pMesh->m_SupportedFlags |= VertexFlags::TEXCOORD;
		if(!(pAssimpMesh->mNumUVComponents[0] == 2)) LOG("UV dimensions don't match internal layout!", Warning);
		for (size_t i = 0; i < pAssimpMesh->mNumVertices; i++)
		{
			pMesh->m_TexCoords.push_back(vec2(
				pAssimpMesh->mTextureCoords[0][i].x,
				pAssimpMesh->mTextureCoords[0][i].y));
		}
	}

	pMesh->CalculateBoundingVolumes();

	delete pImporter;
	return pMesh;
}

MeshFilter* MeshFilterLoader::LoadGLTF(const std::vector<uint8>& binaryContent, const std::string &ext)
{
	glTF::glTFAsset asset;
	if (!glTF::ParseGLTFData(binaryContent, ext, asset))
	{
		LOG("failed to load the glTF asset", Warning);
		return nullptr;
	}

	std::vector<MeshFilter*> filters;
	if (!glTF::MeshFilterConstructor::GetMeshFilters(asset, filters))
	{
		LOG("failed to construct mesh filters from glTF", Warning);
		return nullptr;
	}
	
	if (filters.size() == 0)
	{
		LOG("no mesh filters found in glTF asset", Warning);
		return nullptr;
	}

	MeshFilter* ret = filters[0];

	if (filters.size() > 1)
	{
		for (uint32 i = 1; i < (uint32)filters.size(); ++i) delete filters[i];
	}

	return ret;
}
