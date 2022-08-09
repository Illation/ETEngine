#include "stdafx.h"
#include "Mesh.h"

#include <EtBuild/EngineVersion.h>

#include <EtCore/Content/AssetRegistration.h>
#include <EtCore/Reflection/Registration.h>
#include <EtCore/IO/BinaryReader.h>

#include <EtRendering/MaterialSystem/MaterialData.h>


namespace et {
namespace render {


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

	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	// create a new vertex array
	m_VertexArray = device->CreateVertexArray();
	device->BindVertexArray(m_VertexArray);

	// link it to the mesh's buffer
	device->BindBuffer(rhi::E_BufferType::Vertex, mesh->GetVertexBuffer());
	device->BindBuffer(rhi::E_BufferType::Index, mesh->GetIndexBuffer());

	//Specify Input Layout
	rhi::AttributeDescriptor::DefineAttributeArray(mesh->GetSupportedFlags(), m_Material->GetLayoutFlags(), m_Material->GetAttributeLocations());

	device->BindVertexArray(0u);
}

//---------------------------------
// MeshSurface::d-tor
//
// Free GPU data
//
MeshSurface::~MeshSurface()
{
	rhi::ContextHolder::GetRenderDevice()->DeleteVertexArray(m_VertexArray);
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
// MeshData::d-tor
//
// Free the GPU buffers for this mesh
//
MeshData::~MeshData()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	device->DeleteBuffer(m_VertexBuffer);
	device->DeleteBuffer(m_IndexBuffer);

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
// MeshAsset::ReadEtMesh
//
// Load mesh data from binary asset content, and place it on the GPU
//
bool MeshAsset::ReadEtMesh(MeshData* const meshData, std::vector<uint8> const& loadData)
{
	core::BinaryReader reader;
	reader.Open(loadData);
	ET_ASSERT(reader.Exists());

	// read header
	//-------------
	if (reader.ReadString(s_Header.size()) != s_Header)
	{
		ET_WARNING("Incorrect binary mesh file header");
		return false;
	}

	std::string const writerVersion = reader.ReadNullString();
	if (writerVersion != build::Version::s_Name)
	{
		// right now there are no older file formats that are not supported
		//ET_TRACE_W(ET_CTX_RENDER, "Mesh data was written by a different engine version: %s", writerVersion.c_str());
	}

	// read mesh info
	//----------------
	uint64 const indexCount = reader.Read<uint64>();
	meshData->m_IndexCount = static_cast<size_t>(indexCount);

	uint64 const vertexCount = reader.Read<uint64>();
	meshData->m_VertexCount = static_cast<size_t>(vertexCount);

	meshData->m_IndexDataType = reader.Read<rhi::E_DataType>();
	meshData->m_SupportedFlags = reader.Read<rhi::T_VertexFlags>();
	meshData->m_BoundingSphere.pos = reader.ReadVector<3, float>();
	meshData->m_BoundingSphere.radius = reader.Read<float>();

	uint64 const iBufferSize = indexCount * static_cast<uint64>(rhi::DataTypeInfo::GetTypeSize(meshData->m_IndexDataType));
	uint64 const vBufferSize = vertexCount * static_cast<uint64>(rhi::AttributeDescriptor::GetVertexSize(meshData->m_SupportedFlags));

	// setup buffers
	//---------------
	uint8 const* const indexData = reader.GetCurrentDataPointer();
	reader.MoveBufferPosition(static_cast<size_t>(iBufferSize));

	uint8 const* const vertexData = reader.GetCurrentDataPointer();

	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	// vertex buffer
	meshData->m_VertexBuffer = device->CreateBuffer();
	device->BindBuffer(rhi::E_BufferType::Vertex, meshData->m_VertexBuffer);
	device->SetBufferData(rhi::E_BufferType::Vertex, static_cast<int64>(vBufferSize), reinterpret_cast<void const*>(vertexData), rhi::E_UsageHint::Static);

	// index buffer 
	meshData->m_IndexBuffer = device->CreateBuffer();
	device->BindBuffer(rhi::E_BufferType::Index, meshData->m_IndexBuffer);
	device->SetBufferData(rhi::E_BufferType::Index, static_cast<int64>(iBufferSize), reinterpret_cast<void const*>(indexData), rhi::E_UsageHint::Static);

	return true;
}

//---------------------------------
// MeshAsset::LoadFromMemory
//
bool MeshAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	m_Data = new MeshData();
	if (!ReadEtMesh(m_Data, data))
	{
		delete m_Data;
		m_Data = nullptr;

		return false;
	}

	return true;
}


} // namespace render
} // namespace et
