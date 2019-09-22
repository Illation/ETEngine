#pragma once
#include <EtCore/Content/AssetPointer.h>

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

	T_FbLoc Get() { return m_GlFrameBuffer; }

	std::vector<TextureData*> GetTextures() { return m_pTextureVec; }

protected:

	virtual void AccessShaderAttributes();

	virtual void UploadDerivedVariables() {}

	AssetPtr<ShaderData> m_pShader;
	std::vector<TextureData*> m_pTextureVec;
	bool m_CaptureDepth = false;
private:

	void GenerateFramebufferTextures();
	void ResizeFramebufferTextures();

	std::string m_ShaderFile;
	GLenum m_Format;
	uint32 m_NumTargets = 1;


	T_FbLoc m_GlFrameBuffer;
	T_RbLoc m_RboDepthStencil;
};

