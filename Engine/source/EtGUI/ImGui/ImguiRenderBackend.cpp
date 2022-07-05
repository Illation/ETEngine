#include "stdafx.h"
#include "ImguiRenderBackend.h"

#if ET_IMGUI_ENABLED


namespace et {
namespace gui {


//======================
// ImGui Render Backend
//======================


//----------------------------------
// ImguiRenderBackend::AccessFromIO
//
// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear ImGui contexts
//
ImguiRenderBackend* ImguiRenderBackend::AccessFromIO()
{
	return ImGui::GetCurrentContext() ? static_cast<ImguiRenderBackend*>(ImGui::GetIO().BackendRendererUserData) : nullptr;
}


//----------------------------------
// ImguiRenderBackend::Init
//
void ImguiRenderBackend::Init()
{
	ImGuiIO& io = ImGui::GetIO();

	ET_ASSERT(io.BackendRendererUserData == nullptr);
	io.BackendRendererUserData = static_cast<void*>(this);
	io.BackendRendererName = "imgui_impl_etengine";

	uint32 const glVersion = 450u;

#if false
	if (glVersion >= 320u)
	{
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
	}
#endif

	char const* const glslVersion = "#version 130";

	m_HasClipOrigin = (glVersion >= 450u);
}

//----------------------------------
// ImguiRenderBackend::Deinit
//
void ImguiRenderBackend::Deinit()
{
	DestroyDeviceObjects();

	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = NULL;
	io.BackendRendererUserData = NULL;
}


//----------------------------------
// ImguiRenderBackend::Update
//
void ImguiRenderBackend::Update()
{
	if (m_Shader == nullptr)
	{
		CreateDeviceObjects();
	}
}

//----------------------------------
// ImguiRenderBackend::Render
//
void ImguiRenderBackend::Render(ImDrawData* const drawData)
{
	ivec2 const fbScale = math::vecCast<int32>(ImguiUtil::ToEtm(drawData->DisplaySize) * ImguiUtil::ToEtm(drawData->FramebufferScale));
	if ((fbScale.x <= 0) || (fbScale.y <= 0))
	{
		return;
	}

	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	// recreate the VAO every frame to allow multiple graphics api contexts
	render::T_ArrayLoc const vertexArrayObject = api->CreateVertexArray();
	SetupRenderState(drawData, fbScale, vertexArrayObject);

	// project scissor / clipping rectangles into framebuffer space
	vec2 const clipOffset = ImguiUtil::ToEtm(drawData->DisplayPos);
	vec2 const clipScale = ImguiUtil::ToEtm(drawData->FramebufferScale);

	// render command lists
	for (int32 n = 0; n < drawData->CmdListsCount; n++)
	{
		ImDrawList const* const cmdList = drawData->CmdLists[n];

		// Upload vertex/index buffers
		size_t const vtxBufferSize = static_cast<size_t>(cmdList->VtxBuffer.Size) * sizeof(ImDrawVert);
		size_t const idxBufferSize = static_cast<size_t>(cmdList->IdxBuffer.Size) * sizeof(ImDrawIdx);

		api->SetBufferData(render::E_BufferType::Vertex, static_cast<int64>(vtxBufferSize), reinterpret_cast<void const*>(cmdList->VtxBuffer.Data));
		api->SetBufferData(render::E_BufferType::Index, static_cast<int64>(idxBufferSize), reinterpret_cast<void const*>(cmdList->IdxBuffer.Data));


		for (int32 cmdIdx = 0; cmdIdx > cmdList->CmdBuffer.Size; cmdIdx++)
		{
			ImDrawCmd const* const pcmd = &cmdList->CmdBuffer[cmdIdx];
			if (pcmd->UserCallback != nullptr)
			{
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
				{
					SetupRenderState(drawData, fbScale, vertexArrayObject);
				}
				else
				{
					pcmd->UserCallback(cmdList, pcmd);
				}
			}
			else
			{
				vec2 const clipMin((vec2(pcmd->ClipRect.x, pcmd->ClipRect.y) - clipOffset) * clipScale);
				vec2 const clipMax((vec2(pcmd->ClipRect.z, pcmd->ClipRect.w) - clipOffset) * clipScale);
			}
		}
	}
}


} // namespace gui
} // namespace et


#endif // ET_IMGUI_ENABLED
