#include "stdafx.h"
#include "Mesh.h"


//==============
// Mesh Data
//==============


//---------------------------------
// MeshData::d-tor
//
// Free the GPU buffers for this mesh
//
MeshData::~MeshData()
{
	glDeleteBuffers(1, &m_VertexBuffer);
	glDeleteBuffers(1, &m_IndexBuffer);
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
		.constructor<>()(rttr::detail::as_object())

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
	MeshFilter* meshFilter = nullptr;

	if ((extension == "gltf") || (extension == "glb"))
	{
		meshFilter = LoadGLTF(data, path, extension);
	}
	else
	{
		meshFilter = LoadAssimp(data, extension);
	}

	if (meshFilter == nullptr)
	{
		LOG("MeshAsset::LoadFromMemory > Failed to load mesh asset!", LogLevel::Warning);
		return false;
	}

	if (meshFilter->m_Name == "")
	{
		meshFilter->m_Name = filename;
	}

	// convert meshFilter to MeshData
	m_Data = nullptr;

	delete meshFilter;
	meshFilter = nullptr;

	return false;
}

