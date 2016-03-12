#include "Cube.hpp"
#include "../../Content/ShaderLoader.hpp"
#include "../../Content/TextureLoader.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../../Base\Context.hpp"
#include "../../Base\Settings.hpp"

//Working singleton Set
#define TIME Context::GetInstance()->pTime
#define CAMERA Context::GetInstance()->pCamera
#define SETTINGS Settings::GetInstance()

void Cube::CreateQuad(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 col)
{
	m_Vertices.push_back(VertPosColTex({ a, col,{ 0.0f, 0.0f } }));
	m_Vertices.push_back(VertPosColTex({ b, col,{ 1.0f, 0.0f } }));
	m_Vertices.push_back(VertPosColTex({ c, col,{ 1.0f, 1.0f } }));
	m_Vertices.push_back(VertPosColTex({ d, col,{ 0.0f, 1.0f } }));
}

Cube::Cube()
{
	glm::vec3 a = { -0.5f, -0.5f, -0.5f }; //Y Is Up
	glm::vec3 b = {  0.5f, -0.5f, -0.5f };
	glm::vec3 c = {  0.5f, -0.5f,  0.5f };
	glm::vec3 d = { -0.5f, -0.5f,  0.5f };
	glm::vec3 e = { -0.5f,  0.5f, -0.5f };
	glm::vec3 f = {  0.5f,  0.5f, -0.5f };
	glm::vec3 g = {  0.5f,  0.5f,  0.5f };
	glm::vec3 h = { -0.5f,  0.5f,  0.5f };

	glm::vec3 col = { 1,  1,  1 };
	CreateQuad(f, e, a, b, col); CreateQuad(h, g, c, d, col);
	CreateQuad(g, f, b, c, col); CreateQuad(h, e, a, d, col);
	CreateQuad(c, d, a, b, col); CreateQuad(g, h, e, f, col);
										 
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

	m_pCubeMat = new CubeMaterial("Resources/sample.png", "Resources/sample2.png");
}
Cube::~Cube()
{
	delete m_pCubeMat;
	m_pCubeMat = nullptr;
	glDeleteBuffers(1, &m_ElementBufferObject);
	glDeleteBuffers(1, &m_VertexBufferObject);
	glDeleteVertexArrays(1, &m_VertexArrayObject);
}

void Cube::Initialize()
{
	m_pCubeMat->Initialize();
	//Vertex Array Object
	glGenVertexArrays(1, &m_VertexArrayObject);
	//Vertex Buffer Object
	glGenBuffers(1, &m_VertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertPosColTex)*m_Vertices.size(), m_Vertices.data(), GL_STATIC_DRAW);
	//Specify Input Layout
	glBindVertexArray(m_VertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexArrayObject);
	m_pCubeMat->SpecifyInputLayout();
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
	m_MatModel = glm::rotate(
		m_MatModel,
		TIME->DeltaTime() * glm::radians(180.0f) *0.4f,
		glm::vec3(0.0f, 1.0f, 0.0f)
		);
}
void Cube::Draw()
{
	//Prepare data
	glBindVertexArray(m_VertexArrayObject);
	m_pCubeMat->UploadVariables(m_MatModel);
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
		
		m_MatModel = glm::scale(glm::translate(m_MatModel, glm::vec3(0, -1, 0)), glm::vec3(1, -1, 1));
		m_pCubeMat->UpdateReflectionAtt(m_MatModel, glm::vec3(0.3f, 0.3f, 0.3f));
	
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); // EBO version

		m_MatModel = glm::scale(glm::translate(m_MatModel, glm::vec3(0, -1, 0)), glm::vec3(1, -1, 1));
		m_pCubeMat->UpdateReflectionAtt(m_MatModel, glm::vec3(1, 1, 1));

	glDisable(GL_STENCIL_TEST);
}