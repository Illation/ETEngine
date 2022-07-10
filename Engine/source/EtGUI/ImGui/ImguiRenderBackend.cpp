#include "stdafx.h"
#include "ImguiRenderBackend.h"

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)

#include <EtCore/Content/ResourceManager.h>


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
	ivec2 const fbScale = math::vecCast<int32>(vec2(drawData->DisplaySize) * vec2(drawData->FramebufferScale));
	if ((fbScale.x <= 0) || (fbScale.y <= 0))
	{
		return;
	}

	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	api->DebugPushGroup("ImGui");

	// recreate the VAO every frame to allow multiple graphics api contexts
	render::T_ArrayLoc vertexArrayObject = api->CreateVertexArray();
	SetupRenderState(drawData, fbScale, vertexArrayObject);

	// project scissor / clipping rectangles into framebuffer space
	vec2 const clipOffset(drawData->DisplayPos);
	vec2 const clipScale(drawData->FramebufferScale);

	// render command lists
	for (int32 n = 0; n < drawData->CmdListsCount; n++)
	{
		ImDrawList const* const cmdList = drawData->CmdLists[n];

		// Upload vertex/index buffers
		int64 const vbSize = static_cast<int64>(cmdList->VtxBuffer.Size) * sizeof(ImDrawVert);
		void const* vtxData = reinterpret_cast<void const*>(cmdList->VtxBuffer.Data);
		if (vbSize > m_VertexBufferSize)
		{
			m_VertexBufferSize = vbSize;
			api->SetBufferData(render::E_BufferType::Vertex, vbSize, vtxData, render::E_UsageHint::Stream);
		}
		else
		{
			void* const p = api->MapBuffer(render::E_BufferType::Vertex, render::E_AccessMode::Write);
			memcpy(p, vtxData, static_cast<size_t>(vbSize));
			api->UnmapBuffer(render::E_BufferType::Vertex);
		}

		int64 const ibSize = static_cast<int64>(cmdList->IdxBuffer.Size) * sizeof(ImDrawIdx);
		void const* idxData = reinterpret_cast<void const*>(cmdList->IdxBuffer.Data);
		if (ibSize > m_IndexBufferSize)
		{
			m_IndexBufferSize = ibSize;
			api->SetBufferData(render::E_BufferType::Index, ibSize, idxData, render::E_UsageHint::Stream);
		}
		else
		{
			void* const p = api->MapBuffer(render::E_BufferType::Index, render::E_AccessMode::Write);
			memcpy(p, idxData, static_cast<size_t>(ibSize));
			api->UnmapBuffer(render::E_BufferType::Index);
		}

		// execute commands
		for (int32 cmdIdx = 0; cmdIdx < cmdList->CmdBuffer.Size; cmdIdx++)
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
				if ((clipMax.x <= clipMin.x) || (clipMax.y <= clipMin.y))
				{
					continue;
				}

				api->SetScissor(ivec2(static_cast<int32>(clipMin.x), static_cast<int32>(fbScale.y - clipMax.y)),
					math::vecCast<int32>(clipMax - clipMin));

				// bind texture, draw
				m_Shader->Upload("uTexture"_hash, pcmd->GetTexID());
				api->DrawElements(render::E_DrawMode::Triangles,
					pcmd->ElemCount,
					sizeof(ImDrawIdx) == 2 ? render::E_DataType::UShort : render::E_DataType::UInt,
					(void*)static_cast<intptr_t>(pcmd->IdxOffset * sizeof(ImDrawIdx)));
			}
		}
	}

	// delete temporary vertex array
	api->DeleteVertexArray(vertexArrayObject);	
	api->BindVertexArray(0u);

	// clean up state
	api->SetScissorEnabled(false);
	api->SetBlendEnabled(false);

	api->DebugPopGroup();
}

//-----------------------------------------
// ImguiRenderBackend::CreateDeviceObjects
//
bool ImguiRenderBackend::CreateDeviceObjects()
{
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	m_Shader = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("Shaders/PostGenericUi.glsl"));

	m_VertexBuffer = api->CreateBuffer();
	m_IndexBuffer = api->CreateBuffer();

	CreateFontsTexture();

	return true;
}

//------------------------------------------
// ImguiRenderBackend::DestroyDeviceObjects
//
void ImguiRenderBackend::DestroyDeviceObjects()
{
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	if (m_VertexBuffer != 0u)
	{
		api->DeleteBuffer(m_VertexBuffer);
	}

	if (m_IndexBuffer != 0u)
	{
		api->DeleteBuffer(m_IndexBuffer);
	}

	m_Shader = nullptr;

	DestroyFontsTexture();
}

//--------------------------------------
// ImguiRenderBackend::SetupRenderState
//
void ImguiRenderBackend::SetupRenderState(ImDrawData* const drawData, ivec2 const fbScale, render::T_ArrayLoc const vao)
{
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
	api->SetBlendEnabled(true);
	api->SetBlendEquation(render::E_BlendEquation::Add);
	api->SetBlendFunctionSeparate(render::E_BlendFactor::SourceAlpha, 
		render::E_BlendFactor::One, 
		render::E_BlendFactor::OneMinusSourceAlpha,
		render::E_BlendFactor::OneMinusSourceAlpha);
	api->SetCullEnabled(false);
	api->SetDepthEnabled(false);
	api->SetStencilEnabled(false);
	api->SetScissorEnabled(true);

	// Setup viewport, orthographic projection matrix
	api->SetViewport(ivec2(), fbScale);

	float const L = drawData->DisplayPos.x;
	float const R = drawData->DisplayPos.x + drawData->DisplaySize.x;
	float const T = drawData->DisplayPos.y;
	float const B = drawData->DisplayPos.y + drawData->DisplaySize.y;

	mat4 const orthoProjection(math::orthographic(L, R, T, B, 1.f, -1.f));

	api->SetShader(m_Shader.get());
	m_Shader->Upload("uViewProjection"_hash, orthoProjection);
	m_Shader->Upload("uTransform"_hash, mat4()); // identity matrix
	m_Shader->Upload("uTranslation"_hash, vec2()); // no translation needed for imgui

	// vertex array setup
	api->BindVertexArray(vao);

	api->BindBuffer(render::E_BufferType::Vertex, m_VertexBuffer);
	api->BindBuffer(render::E_BufferType::Index, m_IndexBuffer);

	api->SetVertexAttributeArrayEnabled(0, true);
	api->SetVertexAttributeArrayEnabled(1, true);
	api->SetVertexAttributeArrayEnabled(2, true);

	// the order for ImDrawVert and Rml::Vertex uvs and colors are flipped
	api->DefineVertexAttributePointer(0, 2, render::E_DataType::Float, false, sizeof(ImDrawVert), offsetof(ImDrawVert, pos));
	api->DefineVertexAttributePointer(2, 2, render::E_DataType::Float, false, sizeof(ImDrawVert), offsetof(ImDrawVert, uv)); 
	api->DefineVertexAttributePointer(1, 4, render::E_DataType::UByte, true, sizeof(ImDrawVert), offsetof(ImDrawVert, col));
}

//----------------------------------------
// ImguiRenderBackend::CreateFontsTexture
//
bool ImguiRenderBackend::CreateFontsTexture()
{
	ImGuiIO& io = ImGui::GetIO();

	// Build texture atlas
	uint8* pixels;
	ivec2 dimensions;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &dimensions.x, &dimensions.y); 
	// could get as Alpha8 to save texture space but it would work worse with generic shader

	render::TextureParameters texParams;
	texParams.minFilter = render::E_TextureFilterMode::Linear;
	texParams.magFilter = render::E_TextureFilterMode::Linear;
	texParams.mipFilter = render::E_TextureFilterMode::Nearest;
	texParams.wrapS = render::E_TextureWrapMode::ClampToEdge;
	texParams.wrapT = render::E_TextureWrapMode::ClampToEdge;
	texParams.borderColor = vec4(0.f);
	texParams.genMipMaps = false;

	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();
	api->SetPixelUnpackAlignment(0);

	m_FontTexture = Create<render::TextureData>(render::E_ColorFormat::RGBA8, dimensions);
	m_FontTexture->UploadData(pixels, render::E_ColorFormat::RGBA, render::E_DataType::UByte, 0u);
	m_FontTexture->SetParameters(texParams);

	// Store our identifier
	io.Fonts->SetTexID(m_FontTexture.Get());

	api->SetPixelUnpackAlignment(4);

	return true;
}

//-----------------------------------------
// ImguiRenderBackend::DestroyFontsTexture
//
void ImguiRenderBackend::DestroyFontsTexture()
{
	m_FontTexture = nullptr;
	ImGui::GetIO().Fonts->SetTexID(nullptr);
}


} // namespace gui
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)
