#include <EtEditor/stdafx.h>
#include "MeshDataContainer.h"

#include <ext-mikktspace/mikktspace.h>

#include <EtBuild/EngineVersion.h>

#include <EtCore/IO/BinaryWriter.h>

#include <EtRendering/GraphicsTypes/Mesh.h>


namespace et {
namespace edit {


//=====================
// Mesh Data Container
//=====================


size_t const MeshDataContainer::s_InvalidIndex = std::numeric_limits<size_t>::max();


//--------------------------------------------
// MeshDataContainer::RemoveDuplicateVertices
//
void MeshDataContainer::RemoveDuplicateVertices()
{
	// setup
	MeshDataContainer temp;
	render::T_VertexFlags const localFlags = GetFlags();

	temp.m_Positions.reserve(m_Positions.size());
	temp.m_Normals.reserve(m_Normals.size());
	temp.m_BiNormals.reserve(m_BiNormals.size());
	temp.m_Tangents.reserve(m_Tangents.size());
	temp.m_Colors.reserve(m_Colors.size());
	temp.m_TexCoords.reserve(m_TexCoords.size());

	temp.m_Indices.reserve(m_Indices.size());

	// iterate all vertices
	for (size_t idx = 0u; idx < m_Indices.size(); ++idx)
	{
		size_t index = static_cast<size_t>(m_Indices[idx]);

		size_t foundIdx = temp.GetVertexIdx(*this, index);
		if (foundIdx != s_InvalidIndex) // add existing index
		{
			temp.m_Indices.push_back(static_cast<uint32>(foundIdx));
		}
		else // or copy new vertex and index that
		{
			if (localFlags & render::E_VertexFlag::POSITION) 
			{
				temp.m_Positions.push_back(m_Positions[index]);
			}

			if (localFlags & render::E_VertexFlag::NORMAL) 
			{
				temp.m_Normals.push_back(m_Normals[index]);
			}

			if (localFlags & render::E_VertexFlag::BINORMAL) 
			{
				temp.m_BiNormals.push_back(m_BiNormals[index]);
			}

			if (localFlags & render::E_VertexFlag::TANGENT)
			{
				temp.m_Tangents.push_back(m_Tangents[index]);
			}

			if (localFlags & render::E_VertexFlag::COLOR) 
			{
				temp.m_Colors.push_back(m_Colors[index]);
			}

			if (localFlags & render::E_VertexFlag::TEXCOORD) 
			{
				temp.m_TexCoords.push_back(m_TexCoords[index]);
			}

			temp.m_Indices.push_back(temp.m_VertexCount++);
		}
	}

	// swap into this
	m_VertexCount = temp.m_VertexCount;

	m_Positions.swap(temp.m_Positions);
	m_Normals.swap(temp.m_Normals);
	m_BiNormals.swap(temp.m_BiNormals);
	m_Tangents.swap(temp.m_Tangents);
	m_Colors.swap(temp.m_Colors);
	m_TexCoords.swap(temp.m_TexCoords);

	m_Indices.swap(temp.m_Indices);
}

//--------------------------------
// MeshDataContainer::Triangulate
//
// Discard lines and points
// keep triangles
// triangulate quads
// return false if ngons are found
//
bool MeshDataContainer::Triangulate(std::vector<uint8> const& vcounts)
{
	// setup
	ET_ASSERT(m_Positions.size() == m_VertexCount);

	std::vector<uint32> tempIndices;
	tempIndices.reserve(m_Indices.size());

	// iterate faces
	size_t idx = 0u; // index into original index buffer
	for (uint8 const vcount : vcounts)
	{
		if (vcount < 3u)
		{
			idx += static_cast<size_t>(vcount); // #todo: remove unreferenced vertices in a second step
			continue;
		}

		if (vcount == 3u)
		{
			for (size_t i = idx; i < idx + static_cast<size_t>(vcount); ++i)
			{
				tempIndices.push_back(m_Indices[i]);
			}
		}
		else if (vcount == 4u)
		{
			uint32 a = m_Indices[idx];
			uint32 b = m_Indices[idx + 1u];
			uint32 c = m_Indices[idx + 2u];
			uint32 d = m_Indices[idx + 3u];

			// split along the shorter edge
			if (math::distanceSquared(m_Positions[a], m_Positions[c]) >= math::distanceSquared(m_Positions[b], m_Positions[d]))
			{
				tempIndices.push_back(a);
				tempIndices.push_back(b);
				tempIndices.push_back(d);

				tempIndices.push_back(b);
				tempIndices.push_back(c);
				tempIndices.push_back(d);
			}
			else
			{
				tempIndices.push_back(a);
				tempIndices.push_back(b);
				tempIndices.push_back(c);

				tempIndices.push_back(a);
				tempIndices.push_back(c);
				tempIndices.push_back(d);
			}
		}
		else
		{
			ET_LOG_W(ET_CTX_EDITOR, "Triangulation of NGons is currently not supported, aborting");
			return false;
		}

		idx += static_cast<size_t>(vcount);
	}

	// swap into this
	m_Indices.swap(tempIndices);
	return true;
}

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
			ET_LOG_W(ET_CTX_EDITOR, "Number of texcoords, normals and positions of vertices should match to create tangent space");
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
			vec3 &vertexNormal = userData->m_Normals[userData->m_Indices[faceIdx * 3 + vertIdx]];

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

		mikkTInterface.m_getNumVerticesOfFace = [](SMikkTSpaceContext const*, int const)
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
			vec2 &texCoord = userData->m_TexCoords[userData->m_Indices[faceIdx * 3 + vertIdx]];

			uv[0] = texCoord[0];
			uv[1] = texCoord[1];
		};

		// set tangent info
		mikkTInterface.m_setTSpaceBasic =
			[](const SMikkTSpaceContext* context, const float tangent[3], const float bitangentSign, const int faceIdx, const int vertIdx)
		{
			MikkTSpaceData* const userData = static_cast<MikkTSpaceData*>(context->m_pUserData);

			uint32 const idx = faceIdx * 3u + vertIdx;
			if (idx >= userData->tangents.size())
			{
				userData->tangents.resize(idx + 1u);
			}

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
			ET_LOG_W(ET_CTX_EDITOR, "Failed to generate MikkTSpace tangents");
			return false;
		}
	}

	// validate we have everything we need for bitangents
	if (tangentInfo.size() < m_Positions.size())
	{
		ET_LOG_W(ET_CTX_EDITOR, "Mesh Tangent info size doesn't cover all vertices");
	}

	// generate binormals from tangents and emplace both in the MeshDataContainer
	//----------------------------------------------------------------------------
	ET_ASSERT(m_Tangents.empty());
	ET_ASSERT(m_BiNormals.empty());

	// if we use an index buffer and tangents are generated, tangents wont be properly indexed
	if (!(tangentInfo.size() == m_Normals.size()))
	{
		if (tangentInfo.size() == m_Indices.size())
		{
			std::vector<bool> hasTangent(m_VertexCount, false); // not very classy, suggestions welcome
			m_Tangents.resize(m_VertexCount);
			m_BiNormals.resize(m_VertexCount);

			for (size_t idx = 0u; idx < m_Indices.size(); ++idx)
			{
				uint32 const index = m_Indices[idx];

				if (!hasTangent[index])
				{
					m_Tangents[index] = tangentInfo[idx].xyz;
					m_BiNormals[index] = math::cross(m_Normals[index], tangentInfo[idx].xyz) * tangentInfo[idx].w;
					hasTangent[index] = true;
				}
			}

			return true;
		}

		ET_LOG_W(ET_CTX_EDITOR, "Mesh Tangent info size doesn't match the number of normals");
		return false;
	}

	for (uint32 i = 0; i < tangentInfo.size(); ++i)
	{
		m_Tangents.push_back(tangentInfo[i].xyz);
		m_BiNormals.push_back(math::cross(m_Normals[i], tangentInfo[i].xyz) * tangentInfo[i].w);
	}

	return true;
}

//----------------------------------
// MeshDataContainer::WriteToEtMesh
//
// write into to EtMesh file
//
void MeshDataContainer::WriteToEtMesh(std::vector<uint8>& outData) const
{
	// fetch info so we can calculate file size
	//-------------------------------------------
	uint64 const indexCount = static_cast<uint64>(m_Indices.size());
	uint64 const vertexCount = static_cast<uint64>(m_VertexCount);

	// #todo: might be okay to store index buffer with 16bits per index
	render::E_DataType const indexDataType = render::E_DataType::UInt;
	render::T_VertexFlags const flags = GetFlags();
	math::Sphere const boundingSphere = GetBoundingSphere();

	size_t const iBufferSize = indexCount * static_cast<size_t>(render::DataTypeInfo::GetTypeSize(indexDataType));
	size_t const vBufferSize = vertexCount * static_cast<size_t>(render::AttributeDescriptor::GetVertexSize(flags));

	// init binary writer
	//--------------------
	core::BinaryWriter binWriter(outData);
	binWriter.FormatBuffer(render::MeshAsset::s_Header.size() +
		build::Version::s_Name.size() + 1u +
		sizeof(uint64) + // index count
		sizeof(uint64) + // vertex count
		sizeof(render::E_DataType) +
		sizeof(render::T_VertexFlags) +
		sizeof(float) * 4u + // bounding sphere - pos (3) + radius (1)
		iBufferSize +
		vBufferSize);

	// write header
	//--------------
	binWriter.WriteString(render::MeshAsset::s_Header);
	binWriter.WriteNullString(build::Version::s_Name);

	binWriter.Write(indexCount);
	binWriter.Write(vertexCount);

	binWriter.Write(indexDataType);
	binWriter.Write(flags);
	binWriter.WriteVector(boundingSphere.pos);
	binWriter.Write(boundingSphere.radius);

	// writer indices
	//----------------
	// we just assume index data type will be the same as what is in the mesh container for now.. in the future we might have to convert
	binWriter.WriteData(reinterpret_cast<uint8 const*>(m_Indices.data()), iBufferSize);

	// write vertices
	//----------------
	for (size_t vertIdx = 0u; vertIdx < static_cast<size_t>(vertexCount); vertIdx++)
	{
		if (flags & render::E_VertexFlag::POSITION)
		{
			binWriter.WriteVector(m_Positions[vertIdx]);
		}

		if (flags & render::E_VertexFlag::NORMAL)
		{
			binWriter.WriteVector(m_Normals[vertIdx]);
		}

		if (flags & render::E_VertexFlag::BINORMAL)
		{
			binWriter.WriteVector(m_BiNormals[vertIdx]);
		}

		if (flags & render::E_VertexFlag::TANGENT)
		{
			binWriter.WriteVector(m_Tangents[vertIdx]);
		}

		if (flags & render::E_VertexFlag::COLOR)
		{
			binWriter.WriteVector(m_Colors[vertIdx]);
		}

		if (flags & render::E_VertexFlag::TEXCOORD)
		{
			binWriter.WriteVector(m_TexCoords[vertIdx]);
		}
	}
}

//-----------------------------
// MeshDataContainer::GetFlags
//
render::T_VertexFlags MeshDataContainer::GetFlags() const
{
	render::T_VertexFlags outFlags = 0u;

	if (m_Positions.size() == m_VertexCount)
	{
		outFlags |= render::E_VertexFlag::POSITION;
	}

	if (m_Normals.size() == m_VertexCount)
	{
		outFlags |= render::E_VertexFlag::NORMAL;
	}

	if (m_BiNormals.size() == m_VertexCount)
	{
		outFlags |= render::E_VertexFlag::BINORMAL;
	}

	if (m_Tangents.size() == m_VertexCount)
	{
		outFlags |= render::E_VertexFlag::TANGENT;
	}

	if (m_Colors.size() == m_VertexCount)
	{
		outFlags |= render::E_VertexFlag::COLOR;
	}

	if (m_TexCoords.size() == m_VertexCount)
	{
		outFlags |= render::E_VertexFlag::TEXCOORD;
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

//---------------------------------
// MeshDataContainer::GetVertexIdx
//
// return the index of a matching local vertex, or s_InvalidIndex if none was found
//
size_t MeshDataContainer::GetVertexIdx(MeshDataContainer const& other, size_t const index) const
{
	render::T_VertexFlags const localFlags = GetFlags();
	ET_ASSERT_PARANOID(localFlags == other.GetFlags());

	for (size_t localIdx = 0u; localIdx < m_VertexCount; ++localIdx)
	{
		if ((localFlags & render::E_VertexFlag::POSITION) && (!math::nearEqualsV(m_Positions[localIdx], other.m_Positions[index])))
		{
			continue;
		}

		if ((localFlags & render::E_VertexFlag::NORMAL) && (!math::nearEqualsV(m_Normals[localIdx], other.m_Normals[index])))
		{
			continue;
		}

		if ((localFlags & render::E_VertexFlag::BINORMAL) && (!math::nearEqualsV(m_BiNormals[localIdx], other.m_BiNormals[index])))
		{
			continue;
		}

		if ((localFlags & render::E_VertexFlag::TANGENT) && (!math::nearEqualsV(m_Tangents[localIdx], other.m_Tangents[index])))
		{
			continue;
		}

		if ((localFlags & render::E_VertexFlag::COLOR) && (!math::nearEqualsV(m_Colors[localIdx], other.m_Colors[index])))
		{
			continue;
		}

		if ((localFlags & render::E_VertexFlag::TEXCOORD) && (!math::nearEqualsV(m_TexCoords[localIdx], other.m_TexCoords[index])))
		{
			continue;
		}

		return localIdx;
	}

	return s_InvalidIndex;
}


} // namespace edit
} // namespace et

