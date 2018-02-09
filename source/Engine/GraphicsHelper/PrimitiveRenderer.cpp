#include "stdafx.hpp"

#include "PrimitiveRenderer.hpp"

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
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}
void primitives::Quad::Draw()
{
	STATE->BindVertexArray(m_VAO);
	STATE->DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	STATE->BindVertexArray(0);
}
void primitives::Quad::Initialize()
{
	GLfloat quadVertices[] = 
	{
		// Positions        Texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	// Setup plane VAO
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	STATE->BindVertexArray(m_VAO);
	STATE->BindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
}

//Unit cube
primitives::Cube::~Cube()
{
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}
void primitives::Cube::Draw()
{
	STATE->BindVertexArray(m_VAO);
	STATE->DrawArrays(GL_TRIANGLES, 0, 36);
	STATE->BindVertexArray(0);
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
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	// Fill buffer
	STATE->BindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Link vertex attributes
	STATE->BindVertexArray(m_VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	STATE->BindBuffer(GL_ARRAY_BUFFER, 0);
	STATE->BindVertexArray(0);
}

//Unit sphere with variable detail
template<int32 level>
primitives::IcoSphere<level>::~IcoSphere()
{
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}
template<int32 level>
void primitives::IcoSphere<level>::Draw()
{
	STATE->BindVertexArray(m_VAO);
	STATE->DrawArrays(GL_TRIANGLES, 0, m_NumVerts);
	STATE->BindVertexArray(0);
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

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	// Fill buffer
	STATE->BindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
	// Link vertex attributes
	STATE->BindVertexArray(m_VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	STATE->BindBuffer(GL_ARRAY_BUFFER, 0);
	STATE->BindVertexArray(0);
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