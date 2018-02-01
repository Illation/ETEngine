#include "stdafx.hpp"
#include "MeshFilter.hpp"
#include "Material.hpp"
#include <algorithm>

std::map<VertexFlags, AttributeDescriptor> MeshFilter::LayoutAttributes =
{
	{ POSITION,	{ "position",	GL_FLOAT, 3 } },
	{ NORMAL,	{ "normal",		GL_FLOAT, 3 } },
	{ BINORMAL,	{ "binormal",	GL_FLOAT, 3 } },
	{ TANGENT,	{ "tangent",	GL_FLOAT, 3 } },
	{ COLOR,	{ "color",		GL_FLOAT, 3 } },
	{ TEXCOORD,	{ "texcoord",	GL_FLOAT, 2 } }
};

Sphere* MeshFilter::GetBoundingSphere()
{
	return &m_BoundingSphere;
}

MeshFilter::MeshFilter()
{
}
MeshFilter::~MeshFilter()
{
	m_Positions.clear();
	m_Normals.clear();
	m_TexCoords.clear();
	m_Colors.clear();
	m_Indices.clear();
	m_Tangents.clear();
	m_BiNormals.clear();

	for_each(m_Objects.begin(), m_Objects.end(), [](VertexObject& data)
	{
		data.Release();
	});

	m_Objects.clear();
}

const VertexObject& MeshFilter::GetVertexObject(Material* pMaterial)
{
	int32 index = GetVertexObjectId(pMaterial->GetLayoutFlags());
	if (index < 0)
	{
		BuildVertexBuffer(pMaterial);
		return m_Objects.back();
	}
	return m_Objects[index];
}

int32 MeshFilter::GetVertexObjectId(uint32 flags)
{
	for (uint32 i = 0; i<m_Objects.size(); ++i)
	{
		if (m_Objects[i].flags == flags)
			return i;
	}
	return -1;
}

void MeshFilter::BuildVertexBuffer(Material* pMaterial)
{
	uint32 layoutFlags = pMaterial->GetLayoutFlags();
	//Check if VertexBufferInfo already exists with requested InputLayout
	if (GetVertexObjectId(layoutFlags) >= 0)
		return;
	//Determine stride of vertex layout
	uint32 stride = 0;
	for (auto it = LayoutAttributes.begin(); it != LayoutAttributes.end(); ++it)
	{
		if (layoutFlags & it->first)//material requires this data
		{
			if (m_SupportedFlags & it->first)stride += it->second.dataSize;//filter can provide this data
			else
			{
				string FailString = "Failed to build vertex buffer, mesh filter cannot provide required data\n";
				FailString += "required data: "; 
				FailString += PrintFlags(layoutFlags);
				FailString += "\nprovided data: "; 
				FailString += PrintFlags(m_SupportedFlags);FailString += "\n";
				LOGGER::Log(FailString.c_str(), LogLevel::Error);
				return;
			}
		}
	}
	//Initialize datastructure
	std::vector<float> vertices;
	vertices.reserve(m_VertexCount*stride);
	//Add data if material uses attribute
	for (size_t i = 0; i < m_VertexCount; i++)
	{
		if (layoutFlags & VertexFlags::POSITION)
		{
			vertices.push_back(m_Positions[i].x);
			vertices.push_back(m_Positions[i].y);
			vertices.push_back(m_Positions[i].z);
		}
		if (layoutFlags & VertexFlags::NORMAL)
		{
			vertices.push_back(m_Normals[i].x);
			vertices.push_back(m_Normals[i].y);
			vertices.push_back(m_Normals[i].z);
		}
		if (layoutFlags & VertexFlags::BINORMAL)
		{
			vertices.push_back(m_BiNormals[i].x);
			vertices.push_back(m_BiNormals[i].y);
			vertices.push_back(m_BiNormals[i].z);
		}
		if (layoutFlags & VertexFlags::TANGENT)
		{
			vertices.push_back(m_Tangents[i].x);
			vertices.push_back(m_Tangents[i].y);
			vertices.push_back(m_Tangents[i].z);
		}
		if (layoutFlags & VertexFlags::COLOR)
		{
			vertices.push_back(m_Colors[i].x);
			vertices.push_back(m_Colors[i].y);
			vertices.push_back(m_Colors[i].z);
			//vertices.push_back(m_Colors[i][3]);//not sure which variable to choose
		}
		if (layoutFlags & VertexFlags::TEXCOORD)
		{
			vertices.push_back(m_TexCoords[i].x);
			vertices.push_back(m_TexCoords[i].y);
		}
	}

	VertexObject obj;
	glGenVertexArrays(1, &obj.array);
	STATE->BindVertexArray(obj.array);
	//Vertex Buffer Object
	glGenBuffers(1, &obj.buffer);
	STATE->BindBuffer(GL_ARRAY_BUFFER, obj.buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
	//Specify Input Layout
	pMaterial->SpecifyInputLayout();
	//index buffer
	glGenBuffers(1, &obj.index);
	STATE->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(GLuint)*m_Indices.size(),m_Indices.data(),GL_STATIC_DRAW);
	
	//Add flags for later reference
	obj.flags = layoutFlags;
	//Add to VertexObject datastructure
	m_Objects.push_back(obj);
}

void MeshFilter::CalculateBoundingVolumes()
{
	vec3 center = vec3(0);
	for (size_t i = 0; i < m_Positions.size(); i++)
	{
		center = center + m_Positions[i];
	}
	float rcp = 1.f / static_cast<float>(m_Positions.size());
	center = center * rcp;
	float maxRadius = 0;
	for (size_t i = 0; i < m_Positions.size(); i++)
	{
		float dist = etm::distanceSquared(center, m_Positions[i]);
		if (dist > maxRadius)maxRadius = dist;
	}
	m_BoundingSphere = Sphere(center, sqrtf(maxRadius));
}

std::string MeshFilter::PrintFlags(uint32 flags)
{
	std::string flagstring;
	if (flags & VertexFlags::POSITION)//material requires this data
	{
		flagstring += "POSITION, ";
	}
	if (flags & VertexFlags::NORMAL)//material requires this data
	{
		flagstring += "NORMAL, ";
	}
	if (flags & VertexFlags::BINORMAL)//material requires this data
	{
		flagstring += "BINORMAL, ";
	}
	if (flags & VertexFlags::TANGENT)//material requires this data
	{
		flagstring += "TANGENT, ";
	}
	if (flags & VertexFlags::COLOR)//material requires this data
	{
		flagstring += "COLOR, ";
	}
	if (flags & VertexFlags::TEXCOORD)//material requires this data
	{
		flagstring += "TEXCOORD, ";
	}
	return flagstring;
}