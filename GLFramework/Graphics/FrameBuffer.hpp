#pragma once
#include "../StaticDependancies/glad/glad.h"

class ShaderData;
class TextureData;

class FrameBuffer
{
public:
	FrameBuffer(std::string shaderFile, GLenum format, unsigned numTargets = 1);
	virtual ~FrameBuffer();

	void Initialize();
	void Enable(bool active = true);
	void Draw();

	GLuint Get() { return m_GlFrameBuffer; }

protected:

	virtual void AccessShaderAttributes();

	virtual void UploadDerivedVariables() {}

	ShaderData* m_pShader;
	vector<TextureData*> m_pTextureVec;
private:
	// Quad vertices
	GLuint m_VertexArrayObject;
	GLuint m_VertexBufferObject;

	std::string m_ShaderFile;
	GLenum m_Format;
	unsigned m_NumTargets = 1;

	GLuint m_GlFrameBuffer;
	GLuint m_RboDepthStencil;
};

