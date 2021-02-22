#include "stdafx.h"
#include "MeshDataContainer.h"

#include <ext-mikktspace/mikktspace.h>


namespace et {
namespace pl {


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


} // namespace pl
} // namespace et

