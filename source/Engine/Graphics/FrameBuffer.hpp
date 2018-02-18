#pragma once
#include "../StaticDependancies/glad/glad.h"

class ShaderData;
class TextureData;

class FrameBuffer
{
public:
	FrameBuffer(std::string shaderFile, GLenum format, uint32 numTargets = 1);
	virtual ~FrameBuffer();

	void Initialize();
	void Enable(bool active = true);
	void Draw();

	GLuint Get() { return m_GlFrameBuffer; }

	std::vector<TextureData*> GetTextures() { return m_pTextureVec; }

protected:

	virtual void AccessShaderAttributes();

	virtual void UploadDerivedVariables() {}

	ShaderData* m_pShader;
	std::vector<TextureData*> m_pTextureVec;
	bool m_CaptureDepth = false;
private:

	void GenerateFramebufferTextures();
	void ResizeFramebufferTextures();

	std::string m_ShaderFile;
	GLenum m_Format;
	uint32 m_NumTargets = 1;

	GLuint m_GlFrameBuffer;
	GLuint m_RboDepthStencil;
};

