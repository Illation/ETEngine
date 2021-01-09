#include "stdafx.h"
#include "Mesh.h"

#include <ext-mikktspace/mikktspace.h>

#include <EtBuild/EngineVersion.h>

#include <EtCore/Content/AssetRegistration.h>
#include <EtCore/Reflection/Registration.h>
#include <EtCore/IO/BinaryReader.h>

#include <EtRendering/MaterialSystem/MaterialData.h>


namespace et {
namespace render {


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
			LOG("Number of texcoords, normals and positions of vertices should match to create tangent space", core::LogLevel::Warning);
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
			LOG("Failed to generate MikkTSpace tangents", core::LogLevel::Warning);
			return false;
		}
	}

	// validate we have everything we need for bitangents
	if (tangentInfo.size() < m_Positions.size())
	{
		LOG("Mesh Tangent info size doesn't cover all vertices", core::LogLevel::Warning);
	}

	if (!(tangentInfo.size() == m_Normals.size()))
	{
		LOG("Mesh Tangent info size doesn't match the number of normals", core::LogLevel::Warning);
		return false;
	}

	ET_ASSERT(m_Tangents.empty());
	ET_ASSERT(m_BiNormals.empty());

	// generate binormals from tangents and emplace both in the MeshDataContainer
	//----------------------------------------------------------------------------
	for (uint32 i = 0; i < tangentInfo.size(); ++i)
	{
		m_Tangents.push_back(tangentInfo[i].xyz);
		m_BiNormals.push_back(math::cross(m_Normals[i], tangentInfo[i].xyz) * tangentInfo[i].w);
	}

	return true;
}

//-----------------------------
// MeshDataContainer::GetFlags
//
T_VertexFlags MeshDataContainer::GetFlags() const
{
	T_VertexFlags outFlags = 0u;

	if (m_Positions.size() == m_VertexCount)
	{
		outFlags |= E_VertexFlag::POSITION;
	}

	if (m_Normals.size() == m_VertexCount)
	{
		outFlags |= E_VertexFlag::NORMAL;
	}

	if (m_BiNormals.size() == m_VertexCount)
	{
		outFlags |= E_VertexFlag::BINORMAL;
	}

	if (m_Tangents.size() == m_VertexCount)
	{
		outFlags |= E_VertexFlag::TANGENT;
	}

	if (m_Colors.size() == m_VertexCount)
	{
		outFlags |= E_VertexFlag::COLOR;
	}

	if (m_TexCoords.size() == m_VertexCount)
	{
		outFlags |= E_VertexFlag::TEXCOORD;
	}

	return outFlags;
}

//--------------------------------------
// MeshDataContainer::GetBoundingSphere
//
math::Sphere MeshDataContainer::GetBoundingSphere() const
{
	vec3 center = vec3(0);
	for (size_t i = 0u; i < m_Positions.size(); i++)
	{
		center = center + m_Positions[i];
	}

	float rcp = 1.f / static_cast<float>(m_Positions.size());
	center = center * rcp;

	// greatest distance from center
	float maxRadius = 0.f;
	for (size_t i = 0u; i < m_Positions.size(); i++)
	{
		float dist = math::distanceSquared(center, m_Positions[i]);
		if (dist > maxRadius)maxRadius = dist;
	}

	return math::Sphere(center, sqrtf(maxRadius));
}


//==============
// Mesh Surface
//==============


//---------------------------------
// MeshSurface::c-tor
//
// Construct a surface from a mesh and material combination
//
MeshSurface::MeshSurface(MeshData const* const mesh, render::Material const* const material)
	: m_Material(material)
{
	ET_ASSERT(mesh != nullptr);
	ET_ASSERT(m_Material != nullptr);

	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

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
	ContextHolder::GetRenderContext()->DeleteVertexArray(m_VertexArray);
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
MeshSurface const* SurfaceContainer::GetSurface(MeshData const* const mesh, render::Material const* const material)
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
MeshData::MeshData()
	: m_Surfaces(new SurfaceContainer())
{ }

//---------------------------------
// MeshData::c-tor
//
// Construct Mesh data from vertex data
//
MeshData::MeshData(MeshDataContainer const* const cpuData)
{
	ET_ASSERT(cpuData != nullptr);

	m_IndexCount = cpuData->m_Indices.size();
	m_VertexCount = cpuData->m_VertexCount;
	ET_ASSERT(m_VertexCount > 0u, "Expected mesh to have vertices!");

	// Surface container will contain vertex arrays per material
	m_Surfaces = new SurfaceContainer();

	// derive flags from data sizes
	m_SupportedFlags = cpuData->GetFlags();
	
	// calculate bounding sphere
	m_BoundingSphere = cpuData->GetBoundingSphere();

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

	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

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
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	api->DeleteBuffer(m_VertexBuffer);
	api->DeleteBuffer(m_IndexBuffer);

	delete m_Surfaces;
}

//---------------------------------
// MeshData::GetSurface
//
// Retrieves (and potentially creates) a new surface for the material in question
//
MeshSurface const* MeshData::GetSurface(render::Material const* const material) const
{
	return m_Surfaces->GetSurface(this, material);
}


//===================
// Mesh Asset
//===================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS_ASSET(MeshData, "mesh data")
	END_REGISTER_CLASS(MeshData);

	BEGIN_REGISTER_CLASS(MeshAsset, "mesh asset")
	END_REGISTER_CLASS_POLYMORPHIC(MeshAsset, core::I_Asset);
}
DEFINE_FORCED_LINKING(MeshAsset) // force the shader class to be linked as it is only used in reflection


// static
std::string const MeshAsset::s_Header("ETMESH");


//---------------------------------
// MeshAsset::LoadFromMemory
//
// Load mesh data from binary asset content, and place it on the GPU
//
bool MeshAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	m_Data = new MeshData();

	core::BinaryReader reader;
	reader.Open(data);
	ET_ASSERT(reader.Exists());

	// read header
	//-------------
	if (reader.ReadString(s_Header.size()) != s_Header)
	{
		ET_ASSERT(false, "Incorrect binary mesh file header");
		return false;
	}

	std::string const writerVersion = reader.ReadNullString();
	if (writerVersion != build::Version::s_Name)
	{
		LOG(FS("Mesh data was writter by a different engine version: %s", writerVersion.c_str()));
	}

	// read mesh info
	//----------------
	uint64 const indexCount = reader.Read<uint64>();
	m_Data->m_IndexCount = static_cast<size_t>(indexCount);

	uint64 const vertexCount = reader.Read<uint64>();
	m_Data->m_VertexCount = static_cast<size_t>(vertexCount);

	m_Data->m_IndexDataType = reader.Read<E_DataType>();
	m_Data->m_SupportedFlags = reader.Read<T_VertexFlags>();
	m_Data->m_BoundingSphere.pos = reader.ReadVector<3, float>();
	m_Data->m_BoundingSphere.radius = reader.Read<float>();

	uint64 const iBufferSize = indexCount * static_cast<uint64>(render::DataTypeInfo::GetTypeSize(m_Data->m_IndexDataType));
	uint64 const vBufferSize = vertexCount * static_cast<uint64>(render::AttributeDescriptor::GetVertexSize(m_Data->m_SupportedFlags));

	// setup buffers
	//---------------
	uint8 const* const indexData = reader.GetCurrentDataPointer();
	reader.MoveBufferPosition(static_cast<size_t>(iBufferSize));

	uint8 const* const vertexData = reader.GetCurrentDataPointer();

	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	// vertex buffer
	m_Data->m_VertexBuffer = api->CreateBuffer();
	api->BindBuffer(E_BufferType::Vertex, m_Data->m_VertexBuffer);
	api->SetBufferData(E_BufferType::Vertex, static_cast<int64>(vBufferSize), reinterpret_cast<void const*>(vertexData), E_UsageHint::Static);

	// index buffer 
	m_Data->m_IndexBuffer = api->CreateBuffer();
	api->BindBuffer(E_BufferType::Index, m_Data->m_IndexBuffer);
	api->SetBufferData(E_BufferType::Index, static_cast<int64>(iBufferSize), reinterpret_cast<void const*>(indexData), E_UsageHint::Static);

	return true;
}


} // namespace render
} // namespace et
