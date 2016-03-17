#pragma once
#include "../StaticDependancies/glad/glad.h"

class ShaderData;
class FrameBuffer
{
public:
	FrameBuffer(std::string shaderFile, GLenum format);
	virtual ~FrameBuffer();

	void Initialize();
	void Enable(bool active = true);
	void Draw();

protected:

	virtual void AccessShaderAttributes() {}

	virtual void UploadDerivedVariables() {}

	ShaderData* m_pShader;
private:
	void DefAttLayout(GLuint shaderProgram);
	// Quad vertices
	GLuint m_VertexArrayObject;
	GLuint m_VertexBufferObject;

	std::string m_ShaderFile;
	GLenum m_Format;

	GLuint m_GlFrameBuffer;
	GLuint m_TexColBuffer;
	GLuint m_RboDepthStencil;
};

