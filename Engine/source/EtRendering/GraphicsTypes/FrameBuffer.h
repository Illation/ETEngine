#pragma once
#include <EtCore/Content/AssetPointer.h>

#include "Shader.h"


namespace et {
namespace render {


class TextureData;


class FrameBuffer
{
public:
	FrameBuffer(std::string shaderFile, uint32 numTargets = 1);
	virtual ~FrameBuffer();

	void Initialize();
	void Enable(bool active = true);
	void Draw();

	T_FbLoc Get() { return m_GlFrameBuffer; }

	std::vector<TextureData*> const& GetTextures() const { return m_pTextureVec; }

protected:

	virtual void AccessShaderAttributes() {}
	virtual void UploadDerivedVariables() {}

	AssetPtr<ShaderData> m_pShader;
	std::vector<TextureData*> m_pTextureVec;
	bool m_CaptureDepth = false;
private:

	void GenerateFramebufferTextures();
	void ResizeFramebufferTextures();

	std::string m_ShaderFile;
	uint32 m_NumTargets = 1;

	T_FbLoc m_GlFrameBuffer;
	T_RbLoc m_RboDepthStencil;

	render::T_ViewportEventCallbackId m_VPCallbackId = render::T_ViewportEventDispatcher::INVALID_ID;
};


} // namespace render
} // namespace et
