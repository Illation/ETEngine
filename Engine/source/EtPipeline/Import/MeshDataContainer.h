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
	bool ConstructTangentSpace(std::vector<vec4>& tangentInfo);
	render::T_VertexFlags GetFlags() const;
	math::Sphere GetBoundingSphere() const;

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
