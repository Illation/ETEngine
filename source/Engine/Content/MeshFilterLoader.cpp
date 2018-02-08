#include "stdafx.hpp"
#include "MeshFilterLoader.hpp"

#include "../Graphics/MeshFilter.hpp"

#include <Importer.hpp>
#include <scene.h>  
#include <postprocess.h>

#define ASSIMP_BUILD_BOOST_WORAROUND

MeshFilterLoader::MeshFilterLoader()
{
}


MeshFilterLoader::~MeshFilterLoader()
{
}

MeshFilter* MeshFilterLoader::LoadContent(const std::string& assetFile)
{
	ivec2 logPos = Logger::GetCursorPosition();
	std::string loadingString = std::string("Loading Mesh: ") + assetFile + " . . .";

	LOG(loadingString + " . . . assimp reading file          ", Info, false, logPos);

	//Get the assimp mesh
	auto pImporter = new Assimp::Importer();
	const aiScene* pAssimpScene = pImporter->ReadFile(assetFile,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		//aiProcess_SortByPType |
		aiProcess_GenSmoothNormals |
		aiProcess_PreTransformVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_FlipUVs |
		aiProcess_OptimizeMeshes |
		aiProcess_OptimizeGraph |
		aiProcess_MakeLeftHanded);
	if (!pAssimpScene)
	{
		LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
		LOG("	loading scene with assimp failed: ", Warning);
		LOG(pImporter->GetErrorString(), Warning);
		return nullptr;
	}
	if (!(pAssimpScene->HasMeshes()))
	{
		LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
		LOG("	scene found empty", Warning);
		return nullptr;
	}
	const aiMesh *pAssimpMesh = pAssimpScene->mMeshes[0];
	if (!(pAssimpMesh->HasPositions()))
	{
		LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
		LOG("	mesh found empty", Warning);
		return nullptr;
	}

	//Create mesh and get header info
	MeshFilter* pMesh = new MeshFilter();
	pMesh->m_Name = pAssimpMesh->mName.C_Str();
	if (pMesh->m_Name == "")
	{
		size_t pointPos = assetFile.find_last_of('.');
		size_t slashPos = assetFile.find_last_of('/');
		pMesh->m_Name = assetFile.substr(slashPos+1, pointPos-(slashPos+1));
	}
	pMesh->m_VertexCount = pAssimpMesh->mNumVertices;
	pMesh->m_IndexCount = pAssimpMesh->mNumFaces * 3;
	//Get positions
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
	//Get indices
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
	//Get normals
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
	//Get tangents and bitangents
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
	//Get vertex colors
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
	//Get texture coordinates
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

	//cleanup
	delete pImporter;
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