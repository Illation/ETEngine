#include "stdafx.h"

#include "PrimitiveRenderer.h"


namespace et {
namespace render {


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
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteBuffer(m_VBO);
	api->DeleteVertexArray(m_VAO);
}
void primitives::Quad::Draw()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->BindVertexArray(m_VAO);
	api->DrawArrays(E_DrawMode::TriangleStrip, 0, 4);
	api->BindVertexArray(0);
}
void primitives::Quad::Initialize()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	float quadVertices[] = 
	{
		// Positions        Texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	// Setup plane VAO
	m_VAO = api->CreateVertexArray();
	m_VBO = api->CreateBuffer();
	api->BindVertexArray(m_VAO);
	api->BindBuffer(E_BufferType::Vertex, m_VBO);
	api->SetBufferData(E_BufferType::Vertex, sizeof(quadVertices), &quadVertices, E_UsageHint::Static);
	api->SetVertexAttributeArrayEnabled(0, true);
	api->DefineVertexAttributePointer(0, 3, E_DataType::Float, false, 5 * sizeof(float), 0);
	api->SetVertexAttributeArrayEnabled(1, true);
	api->DefineVertexAttributePointer(1, 2, E_DataType::Float, false, 5 * sizeof(float), (3 * sizeof(float)));
}

//Unit cube
primitives::Cube::~Cube()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteBuffer(m_VBO);
	api->DeleteVertexArray(m_VAO);
}
void primitives::Cube::Draw()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->BindVertexArray(m_VAO);
	api->DrawArrays(E_DrawMode::Triangles, 0, 36);
	api->BindVertexArray(0);
}
void primitives::Cube::Initialize()
{
	float vertices[] = 
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

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	m_VAO = api->CreateVertexArray();
	m_VBO = api->CreateBuffer();
	// Fill buffer
	api->BindBuffer(E_BufferType::Vertex, m_VBO);
	api->SetBufferData(E_BufferType::Vertex, sizeof(vertices), vertices, E_UsageHint::Static);
	// Link vertex attributes
	api->BindVertexArray(m_VAO);
	api->SetVertexAttributeArrayEnabled(0, true);
	api->DefineVertexAttributePointer(0, 3, E_DataType::Float, false, 8 * sizeof(float), 0);
	api->SetVertexAttributeArrayEnabled(1, true);
	api->DefineVertexAttributePointer(1, 3, E_DataType::Float, false, 8 * sizeof(float), (3 * sizeof(float)));
	api->SetVertexAttributeArrayEnabled(2, true);
	api->DefineVertexAttributePointer(2, 2, E_DataType::Float, false, 8 * sizeof(float), (6 * sizeof(float)));
	api->BindBuffer(E_BufferType::Vertex, 0);
	api->BindVertexArray(0);
}

//Unit sphere with variable detail
template<int32 level>
primitives::IcoSphere<level>::~IcoSphere()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteBuffer(m_VBO);
	api->DeleteVertexArray(m_VAO);
}
template<int32 level>
void primitives::IcoSphere<level>::Draw()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->BindVertexArray(m_VAO);
	api->DrawArrays(E_DrawMode::Triangles, 0, m_NumVerts);
	api->BindVertexArray(0);
}
template<int32 level>
void primitives::IcoSphere<level>::Initialize()
{
	auto ico = math::GetIcosahedronPositions(1);
	auto indices = math::GetIcosahedronIndicesBFC();
	std::vector<vec3> vertices;
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		vec3 a = ico[indices[i]];
		vec3 b = ico[indices[i+1]];
		vec3 c = ico[indices[i+2]];

		SubAndPush(vertices, 0, a, b, c);
	}
	m_NumVerts = (int32)vertices.size();

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	m_VAO = api->CreateVertexArray();
	m_VBO = api->CreateBuffer();
	// Fill buffer
	api->BindBuffer(E_BufferType::Vertex, m_VBO);
	api->SetBufferData(E_BufferType::Vertex, sizeof(vec3)*vertices.size(), vertices.data(), E_UsageHint::Static);
	// Link vertex attributes
	api->BindVertexArray(m_VAO);
	api->SetVertexAttributeArrayEnabled(0, true);
	api->DefineVertexAttributePointer(0, 3, E_DataType::Float, false, 3 * sizeof(float), 0);
	api->BindBuffer(E_BufferType::Vertex, 0);
	api->BindVertexArray(0);
}
template<int32 level>
void primitives::IcoSphere<level>::SubAndPush(std::vector<vec3> &vertices, const int32 lev, const vec3 &a, const vec3 &b, const vec3 &c) const
{
	if (lev < level)
	{
		//find midpoints
		vec3 A = math::normalize((b + c) * 0.5f);
		vec3 B = math::normalize((c + a) * 0.5f);
		vec3 C = math::normalize((a + b) * 0.5f);
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


} // namespace render
} // namespace et
