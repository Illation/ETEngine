#include "stdafx.h"
#include "Mesh.h"

#include "Material.h"

#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>  
#include <assimp/postprocess.h>

#include <ext-mikktspace/mikktspace.h>

#include <EtCore/FileSystem/FileUtil.h>

#include <Engine/Helper/GLTF.h>


//=====================
// Mesh Data Container
//=====================


//-------------------------------------------
// MeshDataContainer::ConstructTangentSpace
//
// Generate tangent info from normals. If no tangents are provided, we use MikkTSpace calculations to generate them from normals and texcoords
//
bool MeshDataContainer::ConstructTangentSpace(std::vector<vec4>& tangentInfo)
{
	// if we have no tangents, generate them
	//=======================================
	if (tangentInfo.size() == 0)
	{
		// validate that we have the prerequesites to generate the tangent space
		//-----------------------------------------------------------------------
		if (!(m_TexCoords.size() == m_Normals.size() && m_Normals.size() == m_Positions.size()))
		{
			LOG("Number of texcoords, normals and positions of vertices should match to create tangent space", Warning);
			return false;
		}

		// setup user data to generate the tangent space from 
		//----------------------------------------------------
		struct MikkTSpaceData
		{
			MikkTSpaceData(MeshDataContainer* container, std::vector<vec4>& tangentInfoVec) 
				: dataContainer(container)
				, tangents(tangentInfoVec) 
			{}

			MeshDataContainer* dataContainer;
			std::vector<vec4>& tangents;
		} localUserData(this, tangentInfo);

		// function interface for the library to access and set data
		//-----------------------------------------------------------
		SMikkTSpaceInterface mikkTInterface;

		// access normals
		mikkTInterface.m_getNormal = [](const SMikkTSpaceContext* context, float normal[3], const int faceIdx, const int vertIdx)
			{
				MeshDataContainer *userData = static_cast<MikkTSpaceData*>(context->m_pUserData)->dataContainer;
				vec3 &vertexNormal = userData->m_Normals[faceIdx * 3 + vertIdx];

				for (uint8 i = 0; i < 3; ++i)
				{
					normal[i] = vertexNormal[i];
				}
			};

		// access indices
		mikkTInterface.m_getNumFaces = [](const SMikkTSpaceContext* context)
			{
				MeshDataContainer *userData = static_cast<MikkTSpaceData*>(context->m_pUserData)->dataContainer;
				return static_cast<int>(userData->m_Indices.size() / 3);
			};

		mikkTInterface.m_getNumVerticesOfFace = [](const SMikkTSpaceContext* context, const int faceIdx) 
			{ 
				return 3; 
			};

		// access positions
		mikkTInterface.m_getPosition = [](const SMikkTSpaceContext* context, float position[3], const int faceIdx, const int vertIdx)
			{
				MeshDataContainer *userData = static_cast<MikkTSpaceData*>(context->m_pUserData)->dataContainer;
				vec3 &vertexPosition = userData->m_Positions[userData->m_Indices[faceIdx * 3 + vertIdx]];

				for (uint8 i = 0; i < 3; ++i)
				{
					position[i] = vertexPosition[i];
				}
			};

		// access texcoords
		mikkTInterface.m_getTexCoord = [](const SMikkTSpaceContext* context, float uv[2], const int faceIdx, const int vertIdx)
			{
				MeshDataContainer *userData = static_cast<MikkTSpaceData*>(context->m_pUserData)->dataContainer;
				vec2 &texCoord = userData->m_TexCoords[faceIdx * 3 + vertIdx];

				uv[0] = texCoord[0];
				uv[1] = texCoord[1];
			};

		// set tangent info
		mikkTInterface.m_setTSpaceBasic = 
			[](const SMikkTSpaceContext* context, const float tangent[3], const float bitangentSign, const int faceIdx, const int vertIdx)
			{
				MikkTSpaceData* const userData = static_cast<MikkTSpaceData*>(context->m_pUserData);

				uint32 const idx = faceIdx * 3u + vertIdx;
				userData->tangents.resize(idx);

				vec4& info = userData->tangents[idx];

				for (uint8 i = 0; i < 3; ++i)
				{
					info[i] = tangent[i];
				}

				info.w = bitangentSign;
			};

		mikkTInterface.m_setTSpace = nullptr;

		// run the mikkt tangent space generation
		//----------------------------------------
		SMikkTSpaceContext mikkTContext;
		mikkTContext.m_pInterface = &mikkTInterface;
		mikkTContext.m_pUserData = static_cast<void*>(&localUserData);

		if (!genTangSpaceDefault(&mikkTContext))
		{
			LOG("Failed to generate MikkTSpace tangents", Warning);
			return false;
		}
	}

	// validate we have everything we need for bitangents
	if (tangentInfo.size() < m_Positions.size())
	{
		LOG("Mesh Tangent info size doesn't cover all vertices", Warning);
	}

	if (!(tangentInfo.size() == m_Normals.size()))
	{
		LOG("Mesh Tangent info size doesn't match the number of normals", Warning);
		return false;
	}

	ET_ASSERT(m_Tangents.empty());
	ET_ASSERT(m_BiNormals.empty());

	// generate binormals from tangents and emplace both in the MeshDataContainer
	//----------------------------------------------------------------------------
	for (uint32 i = 0; i < tangentInfo.size(); ++i)
	{
		m_Tangents.push_back(tangentInfo[i].xyz);
		m_BiNormals.push_back(etm::cross(m_Normals[i], tangentInfo[i].xyz) * tangentInfo[i].w);
	}

	return true;
}


//==============
// Mesh Surface
//==============


//---------------------------------
// MeshSurface::c-tor
//
// Construct a surface from a mesh and material combination
//
MeshSurface::MeshSurface(MeshData const* const mesh, Material const* const material)
	: m_Material(material)
{
	ET_ASSERT(mesh != nullptr);
	ET_ASSERT(m_Material != nullptr);

	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	// create a new vertex array
	m_VertexArray = api->CreateVertexArray();
	api->BindVertexArray(m_VertexArray);

	// link it to the mesh's buffer
	api->BindBuffer(E_BufferType::Vertex, mesh->GetVertexBuffer());
	api->BindBuffer(E_BufferType::Index, mesh->GetIndexBuffer());

	//Specify Input Layout
	AttributeDescriptor::DefineAttributeArray(mesh->GetSupportedFlags(), m_Material->GetLayoutFlags(), m_Material->GetAttributeLocations());

	api->BindVertexArray(0u);
}

//---------------------------------
// MeshSurface::d-tor
//
// Free GPU data
//
MeshSurface::~MeshSurface()
{
	Viewport::GetCurrentApiContext()->DeleteVertexArray(m_VertexArray);
}


//===================
// Surface Container
//===================


//---------------------------------
// SurfaceContainer::d-tor
//
// Delete all surface lists
//
SurfaceContainer::~SurfaceContainer()
{
	for (MeshSurface* surface: m_Surfaces)
	{
		SafeDelete(surface);
	}
}

//---------------------------------
// SurfaceContainer::GetSurface
//
// Returns a pointer to a surface for the material in question. If none is found we create it
//
MeshSurface const* SurfaceContainer::GetSurface(MeshData const* const mesh, Material const* const material)
{
	// try finding the existing surface
	auto surfaceIt = std::find_if(m_Surfaces.begin(), m_Surfaces.end(), [material](MeshSurface* surface)
		{
			ET_ASSERT(surface != nullptr);
			return (surface->GetMaterial() == material);
		});

	// if it isn't found, create one and set the mesh
	if (surfaceIt == m_Surfaces.cend())
	{
		m_Surfaces.emplace_back(new MeshSurface(mesh, material));
		surfaceIt = std::prev(m_Surfaces.end());
	}

	return *surfaceIt;
}


//==============
// Mesh Data
//==============


//---------------------------------
// MeshData::c-tor
//
// Construct Mesh data from vertex data
//
MeshData::MeshData(MeshDataContainer const* const cpuData)
{
	ET_ASSERT(cpuData != nullptr);

	m_Name = cpuData->m_Name;

	m_IndexCount = cpuData->m_Indices.size();
	m_VertexCount = cpuData->m_VertexCount;
	ET_ASSERT(m_VertexCount > 0u, "Expected mesh to have vertices!");

	// Surface container will contain vertex arrays per material
	m_Surfaces = new SurfaceContainer();

	// derive flags from data sizes
	//------------------------------
	if (cpuData->m_Positions.size() == m_VertexCount)
	{
		m_SupportedFlags |= E_VertexFlag::POSITION;
	}
	if (cpuData->m_Normals.size() == m_VertexCount)
	{
		m_SupportedFlags |= E_VertexFlag::NORMAL;
	}
	if (cpuData->m_BiNormals.size() == m_VertexCount)
	{
		m_SupportedFlags |= E_VertexFlag::BINORMAL;
	}
	if (cpuData->m_Tangents.size() == m_VertexCount)
	{
		m_SupportedFlags |= E_VertexFlag::TANGENT;
	}
	if (cpuData->m_Colors.size() == m_VertexCount)
	{
		m_SupportedFlags |= E_VertexFlag::COLOR;
	}
	if (cpuData->m_TexCoords.size() == m_VertexCount)
	{
		m_SupportedFlags |= E_VertexFlag::TEXCOORD;
	}

	// calculate bounding sphere
	//---------------------------

	// get center
	vec3 center = vec3(0);
	for (size_t i = 0u; i < cpuData->m_Positions.size(); i++)
	{
		center = center + cpuData->m_Positions[i];
	}

	float rcp = 1.f / static_cast<float>(cpuData->m_Positions.size());
	center = center * rcp;

	// greatest distance from center
	float maxRadius = 0.f;
	for (size_t i = 0u; i < cpuData->m_Positions.size(); i++)
	{
		float dist = etm::distanceSquared(center, cpuData->m_Positions[i]);
		if (dist > maxRadius)maxRadius = dist;
	}

	m_BoundingSphere = Sphere(center, sqrtf(maxRadius));

	// create interleaved buffer data
	//--------------------------------

	// create memory buffer
	uint16 const vertexSize = AttributeDescriptor::GetVertexSize(m_SupportedFlags);
	size_t const bufferSize = m_VertexCount * static_cast<size_t>(vertexSize);
	uint8* interleaved = new uint8[bufferSize];

	// fill interleaved buffer with vertex data
	for (size_t vertIdx = 0u; vertIdx < m_VertexCount; vertIdx++)
	{
		size_t offset = vertIdx * vertexSize;

		if (m_SupportedFlags & E_VertexFlag::POSITION)
		{
			memcpy(interleaved + offset, &(cpuData->m_Positions[vertIdx]), sizeof(vec3));
			offset += sizeof(vec3);
		}

		if (m_SupportedFlags & E_VertexFlag::NORMAL)
		{
			memcpy(interleaved + offset, &(cpuData->m_Normals[vertIdx]), sizeof(vec3));
			offset += sizeof(vec3);
		}

		if (m_SupportedFlags & E_VertexFlag::BINORMAL)
		{
			memcpy(interleaved + offset, &(cpuData->m_BiNormals[vertIdx]), sizeof(vec3));
			offset += sizeof(vec3);
		}

		if (m_SupportedFlags & E_VertexFlag::TANGENT)
		{
			memcpy(interleaved + offset, &(cpuData->m_Tangents[vertIdx]), sizeof(vec3));
			offset += sizeof(vec3);
		}

		if (m_SupportedFlags & E_VertexFlag::COLOR)
		{
			memcpy(interleaved + offset, &(cpuData->m_Colors[vertIdx].xyz), sizeof(vec3));
			offset += sizeof(vec3);
		}

		if (m_SupportedFlags & E_VertexFlag::TEXCOORD)
		{
			memcpy(interleaved + offset, &(cpuData->m_TexCoords[vertIdx]), sizeof(vec2));
			offset += sizeof(vec2);
		}
	}

	// copy data to GPU
	//------------------

	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	// vertex buffer
	m_VertexBuffer = api->CreateBuffer();
	api->BindBuffer(E_BufferType::Vertex, m_VertexBuffer);
	api->SetBufferData(E_BufferType::Vertex, bufferSize, interleaved, E_UsageHint::Static);

	// index buffer - #todo: might be okay to store index buffer with 16bits per index
	m_IndexBuffer = api->CreateBuffer();
	api->BindBuffer(E_BufferType::Index, m_IndexBuffer);
	api->SetBufferData(E_BufferType::Index, sizeof(uint32) * cpuData->m_Indices.size(), cpuData->m_Indices.data(), E_UsageHint::Static);

	// free CPU side data
	//--------------------
	delete[] interleaved;
	interleaved = nullptr;
}

//---------------------------------
// MeshData::d-tor
//
// Free the GPU buffers for this mesh
//
MeshData::~MeshData()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteBuffer(m_VertexBuffer);
	api->DeleteBuffer(m_IndexBuffer);

	SafeDelete(m_Surfaces);
}

//---------------------------------
// MeshData::GetSurface
//
// Retrieves (and potentially creates) a new surface for the material in question
//
MeshSurface const* MeshData::GetSurface(Material const* const material) const
{
	return m_Surfaces->GetSurface(this, material);
}


//===================
// Mesh Asset
//===================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<MeshAsset>("mesh asset")
		.constructor<MeshAsset const&>()
		.constructor<>()(rttr::detail::as_object());

	rttr::type::register_converter_func([](MeshAsset& asset, bool& ok) -> I_Asset*
	{
		ok = true;
		return new MeshAsset(asset);
	});
}
DEFINE_FORCED_LINKING(MeshAsset) // force the shader class to be linked as it is only used in reflection


//---------------------------------
// MeshAsset::LoadFromMemory
//
// Load mesh data from binary asset content, and place it on the GPU
//
bool MeshAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	std::string const extension = FileUtil::ExtractExtension(GetName());
	MeshDataContainer* meshContainer = nullptr;

	meshContainer = LoadAssimp(data, extension);
	if (meshContainer == nullptr)
	{
		LOG("MeshAsset::LoadFromMemory > Failed to load mesh asset!", LogLevel::Warning);
		return false;
	}

	if (meshContainer->m_Name.empty())
	{
		meshContainer->m_Name = GetName();
	}

	m_Data = new MeshData(meshContainer);

	SafeDelete(meshContainer);
	return true;
}

//---------------------------------
// MeshAsset::LoadAssimp
//
// Convert assimp mesh to a CPU side MeshDataContainer
//
MeshDataContainer* MeshAsset::LoadAssimp(std::vector<uint8> const& data, std::string const& extension)
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
		LOG(FS("Loading scene with assimp failed: %s", assimpImporter.GetErrorString()), LogLevel::Warning);
		return nullptr;
	}

	if (!(assimpScene->HasMeshes()))
	{
		LOG("Assimp scene found empty!", LogLevel::Warning);
		return nullptr;
	}

	aiMesh const* const assimpMesh = assimpScene->mMeshes[0];
	if (!(assimpMesh->HasPositions()))
	{
		LOG("Assimp mesh found empty!", Warning);
		return nullptr;
	}


	// start with minumum mesh data
	//------------------------------

	MeshDataContainer* meshData = new MeshDataContainer();
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
		LOG("MeshAsset::LoadAssimp > No indices found!", LogLevel::Warning);
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
			LOG("UV dimensions don't match internal layout!", Warning);
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
// MeshAsset::LoadGLTF
//
// Convert a gltf asset to a CPU side MeshDataContainer
//
MeshDataContainer* MeshAsset::LoadGLTF(std::vector<uint8> const& data, std::string const& path, std::string const& extension)
{
	glTF::glTFAsset asset;
	if (!glTF::ParseGLTFData(data, path, extension, asset))
	{
		LOG("failed to load the glTF asset", Warning);
		return nullptr;
	}

	std::vector<MeshDataContainer*> containers;
	if (!glTF::GetMeshContainers(asset, containers))
	{
		LOG("failed to construct mesh data containers from glTF", Warning);
		return nullptr;
	}

	if (containers.size() == 0)
	{
		LOG("no mesh data containers found in glTF asset", Warning);
		return nullptr;
	}

	MeshDataContainer* ret = containers[0];

	if (containers.size() > 1)
	{
		for (size_t i = 1u; i < containers.size(); ++i)
		{
			delete containers[i];
		}
	}

	return ret;
}
