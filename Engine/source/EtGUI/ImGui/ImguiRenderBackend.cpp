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

	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	device->DebugPushGroup("ImGui");

	// recreate the VAO every frame to allow multiple graphics api contexts
	rhi::T_ArrayLoc vertexArrayObject = device->CreateVertexArray();
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
			device->SetBufferData(rhi::E_BufferType::Vertex, vbSize, vtxData, rhi::E_UsageHint::Stream);
		}
		else
		{
			void* const p = device->MapBuffer(rhi::E_BufferType::Vertex, rhi::E_AccessMode::Write);
			memcpy(p, vtxData, static_cast<size_t>(vbSize));
			device->UnmapBuffer(rhi::E_BufferType::Vertex);
		}

		int64 const ibSize = static_cast<int64>(cmdList->IdxBuffer.Size) * sizeof(ImDrawIdx);
		void const* idxData = reinterpret_cast<void const*>(cmdList->IdxBuffer.Data);
		if (ibSize > m_IndexBufferSize)
		{
			m_IndexBufferSize = ibSize;
			device->SetBufferData(rhi::E_BufferType::Index, ibSize, idxData, rhi::E_UsageHint::Stream);
		}
		else
		{
			void* const p = device->MapBuffer(rhi::E_BufferType::Index, rhi::E_AccessMode::Write);
			memcpy(p, idxData, static_cast<size_t>(ibSize));
			device->UnmapBuffer(rhi::E_BufferType::Index);
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

				device->SetScissor(ivec2(static_cast<int32>(clipMin.x), static_cast<int32>(fbScale.y - clipMax.y)),
					math::vecCast<int32>(clipMax - clipMin));

				// bind texture, draw
				m_Shader->Upload("uTexture"_hash, pcmd->GetTexID());
				device->DrawElements(rhi::E_DrawMode::Triangles,
					pcmd->ElemCount,
					sizeof(ImDrawIdx) == 2 ? rhi::E_DataType::UShort : rhi::E_DataType::UInt,
					(void*)static_cast<intptr_t>(pcmd->IdxOffset * sizeof(ImDrawIdx)));
			}
		}
	}

	// delete temporary vertex array
	device->DeleteVertexArray(vertexArrayObject);	
	device->BindVertexArray(0u);

	// clean up state
	device->SetScissorEnabled(false);
	device->SetBlendEnabled(false);

	device->DebugPopGroup();
}

//-----------------------------------------
// ImguiRenderBackend::CreateDeviceObjects
//
bool ImguiRenderBackend::CreateDeviceObjects()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	m_Shader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("PostGenericUi.glsl"));

	m_VertexBuffer = device->CreateBuffer();
	m_IndexBuffer = device->CreateBuffer();

	CreateFontsTexture();

	return true;
}

//------------------------------------------
// ImguiRenderBackend::DestroyDeviceObjects
//
void ImguiRenderBackend::DestroyDeviceObjects()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	if (m_VertexBuffer != 0u)
	{
		device->DeleteBuffer(m_VertexBuffer);
	}

	if (m_IndexBuffer != 0u)
	{
		device->DeleteBuffer(m_IndexBuffer);
	}

	m_Shader = nullptr;

	DestroyFontsTexture();
}

//--------------------------------------
// ImguiRenderBackend::SetupRenderState
//
void ImguiRenderBackend::SetupRenderState(ImDrawData* const drawData, ivec2 const fbScale, rhi::T_ArrayLoc const vao)
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
	device->SetBlendEnabled(true);
	device->SetBlendEquation(rhi::E_BlendEquation::Add);
	device->SetBlendFunctionSeparate(rhi::E_BlendFactor::SourceAlpha, 
		rhi::E_BlendFactor::One, 
		rhi::E_BlendFactor::OneMinusSourceAlpha,
		rhi::E_BlendFactor::OneMinusSourceAlpha);
	device->SetCullEnabled(false);
	device->SetDepthEnabled(false);
	device->SetStencilEnabled(false);
	device->SetScissorEnabled(true);

	// Setup viewport, orthographic projection matrix
	device->SetViewport(ivec2(), fbScale);

	float const L = drawData->DisplayPos.x;
	float const R = drawData->DisplayPos.x + drawData->DisplaySize.x;
	float const T = drawData->DisplayPos.y;
	float const B = drawData->DisplayPos.y + drawData->DisplaySize.y;

	mat4 const orthoProjection(math::orthographic(L, R, T, B, 1.f, -1.f));

	device->SetShader(m_Shader.get());
	m_Shader->Upload("uViewProjection"_hash, orthoProjection);
	m_Shader->Upload("uTransform"_hash, mat4()); // identity matrix
	m_Shader->Upload("uTranslation"_hash, vec2()); // no translation needed for imgui

	// vertex array setup
	device->BindVertexArray(vao);

	device->BindBuffer(rhi::E_BufferType::Vertex, m_VertexBuffer);
	device->BindBuffer(rhi::E_BufferType::Index, m_IndexBuffer);

	device->SetVertexAttributeArrayEnabled(0, true);
	device->SetVertexAttributeArrayEnabled(1, true);
	device->SetVertexAttributeArrayEnabled(2, true);

	// the order for ImDrawVert and Rml::Vertex uvs and colors are flipped
	device->DefineVertexAttributePointer(0, 2, rhi::E_DataType::Float, false, sizeof(ImDrawVert), offsetof(ImDrawVert, pos));
	device->DefineVertexAttributePointer(2, 2, rhi::E_DataType::Float, false, sizeof(ImDrawVert), offsetof(ImDrawVert, uv)); 
	device->DefineVertexAttributePointer(1, 4, rhi::E_DataType::UByte, true, sizeof(ImDrawVert), offsetof(ImDrawVert, col));
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

	rhi::TextureParameters texParams;
	texParams.minFilter = rhi::E_TextureFilterMode::Linear;
	texParams.magFilter = rhi::E_TextureFilterMode::Linear;
	texParams.mipFilter = rhi::E_TextureFilterMode::Nearest;
	texParams.wrapS = rhi::E_TextureWrapMode::ClampToEdge;
	texParams.wrapT = rhi::E_TextureWrapMode::ClampToEdge;
	texParams.borderColor = vec4(0.f);
	texParams.genMipMaps = false;

	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();
	device->SetPixelUnpackAlignment(0);

	m_FontTexture = Create<rhi::TextureData>(rhi::E_ColorFormat::RGBA8, dimensions);
	m_FontTexture->UploadData(pixels, rhi::E_ColorFormat::RGBA, rhi::E_DataType::UByte, 0u);
	m_FontTexture->SetParameters(texParams);

	// Store our identifier
	io.Fonts->SetTexID(m_FontTexture.Get());

	device->SetPixelUnpackAlignment(4);

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
