#pragma once
#include "VertexInfo.h"

#include <EtCore/Content/Asset.h>
#include <EtCore/Helper/LinkerUtils.h>


// forward declarations
class Material;
class MeshData;


//---------------------------------
// MeshDataContainer
//
// CPU side vertex and index data
//
struct MeshDataContainer final
{
	bool ConstructTangentSpace(std::vector<vec4>& tangentInfo);

	size_t m_VertexCount = 0u;

	std::vector<vec3> m_Positions;
	std::vector<vec3> m_Normals;
	std::vector<vec3> m_BiNormals;
	std::vector<vec3> m_Tangents;
	std::vector<vec4> m_Colors;
	std::vector<vec2> m_TexCoords;

	std::vector<uint32> m_Indices;

	std::string m_Name;
};


//---------------------------------
// MeshSurface
//
// Render packet / renderable view into a subset of a mesh's vertex information
//
class MeshSurface final
{
public:
	// c-tor d-tor
	//-------------
	MeshSurface(MeshData const* const mesh, Material const* const material);
	~MeshSurface();

	// accessors
	//-----------
	Material const* GetMaterial() const { return m_Material; }
	uint32 GetVertexArray() const { return m_VertexArray; }

	// Data
	///////
private:
	Material const* m_Material = nullptr;
	uint32 m_VertexArray = 0u;
};


//---------------------------------
// SurfaceContainer
//
// List of surfaces per mesh
//
class SurfaceContainer final
{
public:
	// c-tor d-tor
	//-------------
	SurfaceContainer() = default;
	~SurfaceContainer();

	// functionality
	//-------------
	MeshSurface const* GetSurface(MeshData const* const mesh, Material const* const material);

	// Data
	///////
private:
	std::vector<MeshSurface*> m_Surfaces;
};


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
	MeshData(MeshDataContainer const* const cpuData);
	~MeshData();

	// accessors
	//-----------
	std::string const& GetName() const { return m_Name; }
	T_VertexFlags GetSupportedFlags() const { return m_SupportedFlags; }
	Sphere const& GetBoundingSphere() const { return m_BoundingSphere; }
	size_t GetIndexCount() const { return m_IndexCount; }
	E_DataType GetIndexDataType() const { return m_IndexDataType; }
	uint32 GetVertexBuffer() const { return m_VertexBuffer; }
	uint32 GetIndexBuffer() const { return m_IndexBuffer; }
	MeshSurface const* GetSurface(Material const* const material) const;

	// Data
	///////
private:

	std::string m_Name;

	T_VertexFlags m_SupportedFlags = 0u;
	E_DataType m_IndexDataType = E_DataType::UInt;

	Sphere m_BoundingSphere;

	size_t m_VertexCount = 0u;
	size_t m_IndexCount = 0u;

	uint32 m_VertexBuffer = 0u;
	uint32 m_IndexBuffer = 0u;

	SurfaceContainer* m_Surfaces = nullptr; // pointer in order to enable const access
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
	MeshDataContainer* LoadAssimp(std::vector<uint8> const& data, std::string const& extension);
	MeshDataContainer* LoadGLTF(std::vector<uint8> const& data, std::string const& path, std::string const& extension);

	// Data
	///////
public:

	RTTR_ENABLE(Asset<MeshData, false>)
};
