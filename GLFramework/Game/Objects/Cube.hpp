#pragma once
#include "../../StaticDependancies/glad/glad.h"
#include <vector>
#include <chrono>

#include <glm\glm.hpp>

#include "../Materials/CubeMaterial.hpp"

#include "../../SceneGraph/GameObject.hpp"

struct VertPosColTex
{
	glm::vec3 pos;
	glm::vec3 col;
	glm::vec2 tex;
};

class Cube : public GameObject
{
public:
	Cube();
	~Cube();

	void Initialize();
	void Start();
	void Draw();
	void Update();

private:
	void CreateQuad(glm::vec3 a, glm::vec3 b, glm::vec3 c,
		glm::vec3 d, glm::vec3 col);
	//Object
	std::vector<VertPosColTex> m_Vertices;
	std::vector<GLuint> m_Indices;
	GLuint m_VertexArrayObject;
	GLuint m_VertexBufferObject;
	GLuint m_ElementBufferObject;

	CubeMaterial* m_pCubeMat = nullptr;

	//Matrices
	glm::mat4 m_MatModel;
};

