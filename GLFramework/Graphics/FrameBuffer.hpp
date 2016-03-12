#pragma once
#include "../StaticDependancies/glad/glad.h"
class FrameBuffer
{
public:
	FrameBuffer();
	virtual ~FrameBuffer();

	void Initialize();
	void Enable(bool active = true);
	void Draw();

private:
	void DefAttLayout(GLuint shaderProgram);
	// Quad vertices
	GLuint m_VertexArrayObject;
	GLuint m_VertexBufferObject;

	GLuint m_VertexShader, m_FragmentShader;
	GLuint m_ShaderProgram;

	GLuint m_GlFrameBuffer;
	GLuint m_TexColBuffer;
	GLuint m_RboDepthStencil;
};

