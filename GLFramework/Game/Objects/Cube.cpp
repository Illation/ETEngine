#include "stdafx.hpp"

#include "Cube.hpp"

#include "../Materials/DiffuseMaterial.hpp"

void Cube::CreateQuad(vec3 a, vec3 b, vec3 c, vec3 d, vec3 col)
{
	m_Vertices.push_back(VertPosColTex({ a, col,{ 0.0f, 0.0f } }));
	m_Vertices.push_back(VertPosColTex({ b, col,{ 1.0f, 0.0f } }));
	m_Vertices.push_back(VertPosColTex({ c, col,{ 1.0f, 1.0f } }));
	m_Vertices.push_back(VertPosColTex({ d, col,{ 0.0f, 1.0f } }));
}

Cube::Cube()
{
	vec3 a = { -0.5f, -0.5f, -0.5f }; //Y Is Up
	vec3 b = {  0.5f, -0.5f, -0.5f };
	vec3 c = {  0.5f, -0.5f,  0.5f };
	vec3 d = { -0.5f, -0.5f,  0.5f };
	vec3 e = { -0.5f,  0.5f, -0.5f };
	vec3 f = {  0.5f,  0.5f, -0.5f };
	vec3 g = {  0.5f,  0.5f,  0.5f };
	vec3 h = { -0.5f,  0.5f,  0.5f };

	//vec3 col = { 1,  1,  1 };
	CreateQuad(f, e, a, b, vec3(0, 0, -1)); CreateQuad(h, g, c, d, vec3(0, 0, 1));
	CreateQuad(g, f, b, c, vec3(1, 0, 0)); CreateQuad(h, e, a, d, vec3(-1, 0, 0));
	CreateQuad(c, d, a, b, vec3(0, -1, 0)); CreateQuad(g, h, e, f, vec3(0, 1, 0));
										 
	m_Vertices.push_back(VertPosColTex({ {-1.0f, -0.5f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }));
	m_Vertices.push_back(VertPosColTex({ { 1.0f, -0.5f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} }));
	m_Vertices.push_back(VertPosColTex({ { 1.0f, -0.5f,  1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} }));
	m_Vertices.push_back(VertPosColTex({ {-1.0f, -0.5f,  1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} }));

	GLuint offset = 0, dO = 4;
	for (size_t i = 0; i < 7; i++)
	{
		GLuint dataArray[6] = { 0+offset, 1 + offset, 2 + offset, 
			2 + offset, 3 + offset, 0 + offset };
		m_Indices.insert(m_Indices.end(), &dataArray[0], &dataArray[6]);
		offset += dO;
	}

	m_pMat = new DiffuseMaterial("Resources/sample.png");
}
Cube::~Cube()
{
	delete m_pMat;
	m_pMat = nullptr;
	glDeleteBuffers(1, &m_ElementBufferObject);
	glDeleteBuffers(1, &m_VertexBufferObject);
	glDeleteVertexArrays(1, &m_VertexArrayObject);
}

void Cube::Initialize()
{
	m_pMat->Initialize();
	//Vertex Array Object
	glGenVertexArrays(1, &m_VertexArrayObject);
	//Vertex Buffer Object
	glGenBuffers(1, &m_VertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertPosColTex)*m_Vertices.size(), m_Vertices.data(), GL_STATIC_DRAW);
	//Specify Input Layout
	glBindVertexArray(m_VertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexArrayObject);
	m_pMat->SpecifyInputLayout();
	//ElementBuffer (index list)
	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*m_Indices.size(), m_Indices.data(), GL_STATIC_DRAW);
}
void Cube::Start()
{
	//Nothing to do
}
void Cube::Update()
{
	// Calculate transformation
	if (!INPUT->IsKeyboardKeyDown(SDL_SCANCODE_SPACE))
	{
		GetTransform()->RotateEuler(0,
			TIME->DeltaTime() * radians(180.0f) *0.4f, 0);
	}
}
void Cube::Draw()
{
	//Prepare data
	glBindVertexArray(m_VertexArrayObject);
	m_pMat->UploadVariables(GetTransform()->GetWorld());
	// Draw Cube
	glEnable(GL_DEPTH_TEST);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); // EBO version

	glEnable(GL_STENCIL_TEST);
	
		// Draw floor
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF); // Write to stencil buffer
		glDepthMask(GL_FALSE); // Don't write to depth buffer
		glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)
		
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLuint *)NULL + (36)); // EBO version
		
		// Draw cube reflection
		glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
		glStencilMask(0x00); // Don't write anything to stencil buffer
		glDepthMask(GL_TRUE); // Write to depth buffer
		
		//mat4 matModel = scale(translate(GetTransform()->GetWorld(), vec3(0, -1, 0)), vec3(1, -1, 1));
		//m_pCubeMat->UpdateReflectionAtt(matModel, vec3(0.3f, 0.3f, 0.3f));
		
		//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); // EBO version
		//m_pCubeMat->UpdateReflectionAtt(GetTransform()->GetWorld(), vec3(1, 1, 1));
	
	glDisable(GL_STENCIL_TEST);
}