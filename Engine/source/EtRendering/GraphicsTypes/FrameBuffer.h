#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtRHI/GraphicsTypes/Shader.h>


namespace et {
namespace render {


class rhi::TextureData;


class FrameBuffer
{
public:
	FrameBuffer(std::string shaderFile, uint32 numTargets = 1);
	virtual ~FrameBuffer();

	void Initialize();
	void Enable(bool active = true);
	void Draw();

	rhi::T_FbLoc Get() { return m_GlFrameBuffer; }

	std::vector<rhi::TextureData*> const& GetTextures() const { return m_pTextureVec; }

protected:

	virtual void AccessShaderAttributes() {}
	virtual void UploadDerivedVariables() {}

	AssetPtr<rhi::ShaderData> m_pShader;
	std::vector<rhi::TextureData*> m_pTextureVec;
	bool m_CaptureDepth = false;
private:

	void GenerateFramebufferTextures();
	void ResizeFramebufferTextures();

	std::string m_ShaderFile;
	uint32 m_NumTargets = 1;

	rhi::T_FbLoc m_GlFrameBuffer;
	rhi::T_RbLoc m_RboDepthStencil;

	rhi::T_ViewportEventCallbackId m_VPCallbackId = rhi::T_ViewportEventDispatcher::INVALID_ID;
};


} // namespace render
} // namespace et
