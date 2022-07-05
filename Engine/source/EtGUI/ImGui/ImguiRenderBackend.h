#pragma once
#include "ImGui.h"

#if ET_IMGUI_ENABLED


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
	void CreateDeviceObjects();
	void DestroyDeviceObjects();

	void SetupRenderState(ImDrawData* const drawData, ivec2 const fbScale, render::T_ArrayLoc const vao);

	bool CreateFontsTexture();
	void DestroyFontsTexture();


	// Data
	///////

	UniquePtr<render::TextureData> m_FontTexture;
	AssetPtr<render::ShaderData> m_Shader;

	render::T_ArrayLoc m_Vbo = 0u;
	render::T_BufferLoc m_Elements = 0u;

	size_t m_VertexBufferSize = 0u;
	size_t m_IndexBufferSize = 0u;

	bool m_HasClipOrigin = false;
	bool m_UseBufferSubData = false;
};


} // namespace gui
} // namespace et


#endif // ET_IMGUI_ENABLED
