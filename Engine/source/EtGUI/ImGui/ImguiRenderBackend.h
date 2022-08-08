#pragma once
#include <imgui/imgui.h>

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)

#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/GraphicsTypes/TextureData.h>


namespace et {
namespace gui {


//--------------------
// ImguiRenderBackend
//
// Functionality for rendering imgui draw data to the GPU
//
class ImguiRenderBackend final 
{
	// static functionality
	//----------------------
public:
	static ImguiRenderBackend* AccessFromIO();

	// construct destruct
	//--------------------
	ImguiRenderBackend() = default;

	void Init();
	void Deinit();

	// functionality
	//---------------
	void Update();
	void Render(ImDrawData* const drawData);

	// utility
	//---------
private:
	bool CreateDeviceObjects();
	void DestroyDeviceObjects();

	void SetupRenderState(ImDrawData* const drawData, ivec2 const fbScale, rhi::T_ArrayLoc const vao);

	bool CreateFontsTexture();
	void DestroyFontsTexture();


	// Data
	///////

	UniquePtr<rhi::TextureData> m_FontTexture;
	AssetPtr<rhi::ShaderData> m_Shader;

	rhi::T_BufferLoc m_VertexBuffer = 0u;
	rhi::T_BufferLoc m_IndexBuffer = 0u;

	int64 m_VertexBufferSize = 0;
	int64 m_IndexBufferSize = 0;
};


} // namespace gui
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)
