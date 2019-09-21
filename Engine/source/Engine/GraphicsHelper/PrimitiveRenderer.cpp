#include "stdafx.h"

#include <glad/glad.h>

#include "PrimitiveRenderer.h"


//Abstract
//*********

void PrimitiveGeometry::RootDraw()
{
	if (!m_IsInitialized)
	{
		Initialize();
		m_IsInitialized = true;
	}
	Draw();
}

PrimitiveRenderer::PrimitiveRenderer()
{
	AddGeometry(new primitives::Quad());
	AddGeometry(new primitives::Cube());
	AddGeometry(new primitives::IcoSphere<0>());
	AddGeometry(new primitives::IcoSphere<1>());
	AddGeometry(new primitives::IcoSphere<2>());
	AddGeometry(new primitives::IcoSphere<3>());
}
PrimitiveRenderer::~PrimitiveRenderer()
{
	for (PrimitiveGeometry* geometry : m_pTypes)
	{
		SafeDelete(geometry);
	}
	m_pTypes.clear();
}
void PrimitiveRenderer::AddGeometry(PrimitiveGeometry* pGeometry)
{
	for (PrimitiveGeometry* pType : m_pTypes)
	{
		if (pType->GetType() == pGeometry->GetType())
		{
			SafeDelete(pGeometry);
			return;
		}
	}
	m_pTypes.push_back(pGeometry);
}


//Specifications
//***************

//Unit Quad
primitives::Quad::~Quad()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteBuffers(1, &m_VBO);
	api->DeleteVertexArrays(1, &m_VAO);
}
void primitives::Quad::Draw()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->BindVertexArray(m_VAO);
	api->DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	api->BindVertexArray(0);
}
void primitives::Quad::Initialize()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	GLfloat quadVertices[] = 
	{
		// Positions        Texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	// Setup plane VAO
	api->GenerateVertexArrays(1, &m_VAO);
	api->GenerateBuffers(1, &m_VBO);
	api->BindVertexArray(m_VAO);
	api->BindBuffer(GL_ARRAY_BUFFER, m_VBO);
	api->SetBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	api->SetVertexAttributeArrayEnabled(0, true);
	api->DefineVertexAttributePointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	api->SetVertexAttributeArrayEnabled(1, true);
	api->DefineVertexAttributePointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
}

//Unit cube
primitives::Cube::~Cube()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteBuffers(1, &m_VBO);
	api->DeleteVertexArrays(1, &m_VAO);
}
void primitives::Cube::Draw()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->BindVertexArray(m_VAO);
	api->DrawArrays(GL_TRIANGLES, 0, 36);
	api->BindVertexArray(0);
}
void primitives::Cube::Initialize()
{
	GLfloat vertices[] = 
	{
		// Back face
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,  // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,// top-left
		// Front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,  // bottom-right
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,  // top-right
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,  // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,  // bottom-left
		// Left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,  // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		// Right face
		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  // bottom-right
		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  // top-left
		1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
		// Bottom face
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,// bottom-left
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		// Top face
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,// top-left
		1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,// top-left
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f // bottom-left        
	};

	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->GenerateVertexArrays(1, &m_VAO);
	api->GenerateBuffers(1, &m_VBO);
	// Fill buffer
	api->BindBuffer(GL_ARRAY_BUFFER, m_VBO);
	api->SetBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Link vertex attributes
	api->BindVertexArray(m_VAO);
	api->SetVertexAttributeArrayEnabled(0, true);
	api->DefineVertexAttributePointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	api->SetVertexAttributeArrayEnabled(1, true);
	api->DefineVertexAttributePointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	api->SetVertexAttributeArrayEnabled(2, true);
	api->DefineVertexAttributePointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	api->BindBuffer(GL_ARRAY_BUFFER, 0);
	api->BindVertexArray(0);
}

//Unit sphere with variable detail
template<int32 level>
primitives::IcoSphere<level>::~IcoSphere()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteBuffers(1, &m_VBO);
	api->DeleteVertexArrays(1, &m_VAO);
}
template<int32 level>
void primitives::IcoSphere<level>::Draw()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->BindVertexArray(m_VAO);
	api->DrawArrays(GL_TRIANGLES, 0, m_NumVerts);
	api->BindVertexArray(0);
}
template<int32 level>
void primitives::IcoSphere<level>::Initialize()
{
	auto ico = GetIcosahedronPositions(1);
	auto indices = GetIcosahedronIndicesBFC();
	std::vector<vec3> vertices;
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		vec3 a = ico[indices[i]];
		vec3 b = ico[indices[i+1]];
		vec3 c = ico[indices[i+2]];

		SubAndPush(vertices, 0, a, b, c);
	}
	m_NumVerts = (int32)vertices.size();

	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->GenerateVertexArrays(1, &m_VAO);
	api->GenerateBuffers(1, &m_VBO);
	// Fill buffer
	api->BindBuffer(GL_ARRAY_BUFFER, m_VBO);
	api->SetBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
	// Link vertex attributes
	api->BindVertexArray(m_VAO);
	api->SetVertexAttributeArrayEnabled(0, true);
	api->DefineVertexAttributePointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	api->BindBuffer(GL_ARRAY_BUFFER, 0);
	api->BindVertexArray(0);
}
template<int32 level>
void primitives::IcoSphere<level>::SubAndPush(std::vector<vec3> &vertices, const int32 lev, const vec3 &a, const vec3 &b, const vec3 &c) const
{
	if (lev < level)
	{
		//find midpoints
		vec3 A = etm::normalize(b + ((c - b)*0.5f));
		vec3 B = etm::normalize(c + ((a - c)*0.5f));
		vec3 C = etm::normalize(a + ((b - a)*0.5f));
		//Make 4 new triangles
		int32 nLevel = lev + 1;
		SubAndPush(vertices, nLevel, B, A, c);
		SubAndPush(vertices, nLevel, b, A, C);
		SubAndPush(vertices, nLevel, B, a, C);
		SubAndPush(vertices, nLevel, A, B, C);
	}
	else
	{
		vertices.push_back(a);
		vertices.push_back(b);
		vertices.push_back(c);
	}
}