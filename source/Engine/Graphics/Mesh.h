#pragma once
#include "VertexInfo.h"

#include <EtCore/Content/Asset.h>
#include <EtCore/Helper/LinkerUtils.h>


//---------------------------------
// MeshData
//
// Info about GPU mesh data and metadata
//
class MeshData final
{
public:
	// c-tor d-tor
	//-------------
	MeshData() = default;
	~MeshData();

	// accessors
	//-----------
	std::string const& GetName() const { return m_Name; }
	Sphere const& GetBoundingSphere() const { return m_BoundingSphere; }
	size_t GetIndexCount() const { return m_IndexCount; }
	uint32 GetVertexBuffer() const { return m_VertexBuffer; }
	uint32 GetIndexBuffer() const { return m_IndexBuffer; }

	// Data
	///////
private:

	std::string m_Name;

	T_VertexFlags m_SupportedFlags;

	Sphere m_BoundingSphere;

	size_t m_VertexCount;
	size_t m_IndexCount;

	uint32 m_VertexBuffer;
	uint32 m_IndexBuffer;
};


//---------------------------------
// MeshAsset
//
// Loadable Mesh Data
//
class MeshAsset final : public Asset<MeshData, false>
{
	DECLARE_FORCED_LINKING()
public:
	// Construct destruct
	//---------------------
	MeshAsset() : Asset<MeshData, false>() {}
	virtual ~MeshAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	// Data
	///////
public:

	RTTR_ENABLE(Asset<MeshData, false>)
};
