#pragma once


namespace et {
namespace pl {


//---------------------------------
// MeshDataContainer
//
// CPU side vertex and index data
//
struct MeshDataContainer final
{
	// definitions
	//-------------
	static size_t const s_InvalidIndex;

	// functionality
	//---------------
	void RemoveDuplicateVertices();
	bool Triangulate(std::vector<uint8> const& vcounts);
	bool ConstructTangentSpace(std::vector<vec4>& tangentInfo);

	// accessors
	//-----------
	render::T_VertexFlags GetFlags() const;
	math::Sphere GetBoundingSphere() const;

	size_t GetVertexIdx(MeshDataContainer const& other, size_t const index) const;

	// Data
	///////

	std::string m_Name;

	size_t m_VertexCount = 0u;

	std::vector<vec3> m_Positions;
	std::vector<vec3> m_Normals;
	std::vector<vec3> m_BiNormals;
	std::vector<vec3> m_Tangents;
	std::vector<vec4> m_Colors;
	std::vector<vec2> m_TexCoords;

	std::vector<uint32> m_Indices;
};


} // namespace pl
} // namespace et
