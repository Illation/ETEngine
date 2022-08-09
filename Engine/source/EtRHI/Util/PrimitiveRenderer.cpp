#include "stdafx.h"

#include "PrimitiveRenderer.h"


namespace et {
namespace rhi {


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

PrimitiveRenderer& PrimitiveRenderer::Instance()
{
	static PrimitiveRenderer s_PrimitiveRenderer;
	return s_PrimitiveRenderer;
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
	Deinit();
}

void PrimitiveRenderer::Deinit()
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
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	device->DeleteBuffer(m_VBO);
	device->DeleteVertexArray(m_VAO);
}
void primitives::Quad::Draw()
{
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	device->BindVertexArray(m_VAO);
	device->DrawArrays(E_DrawMode::TriangleStrip, 0, 4);
	device->BindVertexArray(0);
}
void primitives::Quad::Initialize()
{
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	float quadVertices[] = 
	{
		// Positions        Texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	// Setup plane VAO
	m_VAO = device->CreateVertexArray();
	m_VBO = device->CreateBuffer();
	device->BindVertexArray(m_VAO);
	device->BindBuffer(E_BufferType::Vertex, m_VBO);
	device->SetBufferData(E_BufferType::Vertex, sizeof(quadVertices), &quadVertices, E_UsageHint::Static);
	device->SetVertexAttributeArrayEnabled(0, true);
	device->DefineVertexAttributePointer(0, 3, E_DataType::Float, false, 5 * sizeof(float), 0);
	device->SetVertexAttributeArrayEnabled(1, true);
	device->DefineVertexAttributePointer(1, 2, E_DataType::Float, false, 5 * sizeof(float), (3 * sizeof(float)));
}

//Unit cube
primitives::Cube::~Cube()
{
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	device->DeleteBuffer(m_VBO);
	device->DeleteVertexArray(m_VAO);
}
void primitives::Cube::Draw()
{
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	device->BindVertexArray(m_VAO);
	device->DrawArrays(E_DrawMode::Triangles, 0, 36);
	device->BindVertexArray(0);
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

	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	m_VAO = device->CreateVertexArray();
	m_VBO = device->CreateBuffer();
	// Fill buffer
	device->BindBuffer(E_BufferType::Vertex, m_VBO);
	device->SetBufferData(E_BufferType::Vertex, sizeof(vertices), vertices, E_UsageHint::Static);
	// Link vertex attributes
	device->BindVertexArray(m_VAO);
	device->SetVertexAttributeArrayEnabled(0, true);
	device->DefineVertexAttributePointer(0, 3, E_DataType::Float, false, 8 * sizeof(float), 0);
	device->SetVertexAttributeArrayEnabled(1, true);
	device->DefineVertexAttributePointer(1, 3, E_DataType::Float, false, 8 * sizeof(float), (3 * sizeof(float)));
	device->SetVertexAttributeArrayEnabled(2, true);
	device->DefineVertexAttributePointer(2, 2, E_DataType::Float, false, 8 * sizeof(float), (6 * sizeof(float)));
	device->BindBuffer(E_BufferType::Vertex, 0);
	device->BindVertexArray(0);
}

//Unit sphere with variable detail
template<int32 level>
primitives::IcoSphere<level>::~IcoSphere()
{
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	device->DeleteBuffer(m_VBO);
	device->DeleteVertexArray(m_VAO);
}
template<int32 level>
void primitives::IcoSphere<level>::Draw()
{
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	device->BindVertexArray(m_VAO);
	device->DrawArrays(E_DrawMode::Triangles, 0, m_NumVerts);
	device->BindVertexArray(0);
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

	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	m_VAO = device->CreateVertexArray();
	m_VBO = device->CreateBuffer();
	// Fill buffer
	device->BindBuffer(E_BufferType::Vertex, m_VBO);
	device->SetBufferData(E_BufferType::Vertex, sizeof(vec3)*vertices.size(), vertices.data(), E_UsageHint::Static);
	// Link vertex attributes
	device->BindVertexArray(m_VAO);
	device->SetVertexAttributeArrayEnabled(0, true);
	device->DefineVertexAttributePointer(0, 3, E_DataType::Float, false, 3 * sizeof(float), 0);
	device->BindBuffer(E_BufferType::Vertex, 0);
	device->BindVertexArray(0);
}
template<int32 level>
void primitives::IcoSphere<level>::SubAndPush(std::vector<vec3> &vertices, const int32 lev, const vec3 &a, const vec3 &b, const vec3 &c) const
{
	if (lev < level)
	{
		//find midpoints
		vec3 A = math::normalize(b + ((c - b)*0.5f));
		vec3 B = math::normalize(c + ((a - c)*0.5f));
		vec3 C = math::normalize(a + ((b - a)*0.5f));
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


} // namespace rhi
} // namespace et
