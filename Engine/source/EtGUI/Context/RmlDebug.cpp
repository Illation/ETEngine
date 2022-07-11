#include "stdafx.h"
#include "RmlDebug.h"

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)

#include "TickOrder.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

#include <EtCore/Util/DebugCommandController.h>

#include <EtGUI/Rendering/RmlRenderer.h>
#include <EtGUI/Fonts/FontEngine.h>


namespace et {
namespace gui {


//============
// RML Debug
//============


//------------------------
// RmlDebug::RmlDebug
//
RmlDebug::RmlDebug() 
	: core::I_Tickable(static_cast<uint32>(E_TickOrder::TICK_GuiDebug))
{
	core::dbg::CommandController& cmdController = core::dbg::CommandController::Instance();

	cmdController.AddCommand(core::dbg::Command("gui_debug", "Show debugging options for the GUI system"), core::dbg::T_CommandFn(
		[this](core::dbg::Command const& command, std::string const& parameters)
		{
			ET_UNUSED(command);
			ET_UNUSED(parameters);
			m_IsVisible = !m_IsVisible;
			return core::dbg::E_CommandRes::Success;
		}));
}

//-----------------
// RmlDebug::Init
//
void RmlDebug::Init(Ptr<RmlRenderer const> const renderer, Ptr<FontEngine const> const fontEngine)
{
	m_Renderer = renderer;
	m_FontEngine = fontEngine;
}

//------------------------
// RmlDebug::OnTick
//
void RmlDebug::OnTick()
{
	if (m_IsVisible)
	{
		if (ImGui::Begin("GUI Debug", &m_IsVisible))
		{
			ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

			ShowContextUI();
			ShowRenderingUI();
		}

		ImGui::End();
	}
}

//---------------------------
// RmlDebug::ShowRenderingUI
//
void RmlDebug::ShowContextUI()
{
	int32 const numContexts = Rml::GetNumContexts();
	if (ImGui::CollapsingHeader(FS("Contexts - %i", numContexts).c_str()))
	{
		for (int32 ctxIdx = 0; ctxIdx < numContexts; ++ctxIdx)
		{
			Rml::Context const* const context = Rml::GetContext(ctxIdx);
			std::string const& contextName = context->GetName();
			if (ImGui::TreeNode(FS("[%i] - %s", ctxIdx, contextName.c_str()).c_str()))
			{

				ImGui::TreePop();
			}
		}
	}
}

//---------------------------
// RmlDebug::ShowRenderingUI
//
void RmlDebug::ShowRenderingUI()
{
	static vec2 const s_MaxTextureScale(150.f);

	ImGui::SetNextItemOpen(true);
	if (ImGui::CollapsingHeader("Rendering"))
	{
		ImGui::Text("Immediate geometry buffer size: %u", m_Renderer->m_VertexBufferSize + m_Renderer->m_IndexBufferSize);

		if (ImGui::TreeNode(FS("Compiled Geometries: " ET_FMT_SIZET, m_Renderer->m_Geometries.size()).c_str()))
		{
			for (auto const& geoPair : m_Renderer->m_Geometries)
			{
				RmlRenderer::Geometry const& geo = geoPair.second;
				if (ImGui::TreeNode(FS("["ET_FMT_SIZET"] - %s", static_cast<size_t>(geoPair.first), (geo.m_Font != nullptr) ? "Font" : "Generic").c_str()))
				{
					ImGui::Text("vertices: %i", geo.m_NumVertices);
					ImGui::Text("indices: %i", geo.m_NumIndices);
					ImGui::Text("instances: %u", geo.m_InstanceCount);

					if ((geo.m_Texture != nullptr) && (geo.m_Texture != m_Renderer->m_EmptyWhiteTex2x2))
					{
						AddOpaqueImage(geo.m_Texture.Get(), s_MaxTextureScale);
					}

					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode(FS("Textures: " ET_FMT_SIZET, m_Renderer->m_Textures.size()).c_str()))
		{
			for (auto const& texPair : m_Renderer->m_Textures)
			{
				RmlRenderer::Texture const& tex = texPair.second;
				if (ImGui::TreeNode(FS("["ET_FMT_SIZET"] - %s", static_cast<size_t>(texPair.first), tex.GetId().ToStringDbg()).c_str()))
				{
					AddOpaqueImage(tex.Get().Get(), s_MaxTextureScale);
					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}
	}
}

//---------------------------
// SetBlendEnabledCallback
//
// Draw list command to set the blend state
//
template <bool TEnabled>
void SetBlendEnabledCallback(ImDrawList const*, ImDrawCmd const*)
{
	render::ContextHolder::GetRenderContext()->SetBlendEnabled(TEnabled);
}

//---------------------------
// RmlDebug::AddOpaqueImage
//
// temporarily disable blending to make sure the images are not drawn as transparent
//
void RmlDebug::AddOpaqueImage(render::TextureData const* const texture, vec2 const maxScale)
{
	vec2 const resolution = math::vecCast<float>(texture->GetResolution());

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddCallback(SetBlendEnabledCallback<false>, nullptr);
	if (ImGui::ImageButton(texture, GetScale(resolution, maxScale)))
	{
		ImGui::OpenPopup("full_image_popup");
	}

	drawList->AddCallback(SetBlendEnabledCallback<true>, nullptr);

	if (ImGui::BeginPopup("full_image_popup"))
	{
		vec2 const pos = ImGui::GetCursorScreenPos();
		vec2 const popScale = GetScale(resolution, vec2(ImGui::GetMainViewport()->WorkSize) - vec2(ImGui::GetStyle().FramePadding) - vec2(10.f));

		drawList = ImGui::GetForegroundDrawList();
		drawList->AddCallback(SetBlendEnabledCallback<false>, nullptr);
		drawList->AddImage(texture, pos, pos + popScale);
		drawList->AddCallback(SetBlendEnabledCallback<true>, nullptr);

		ImGui::Dummy(popScale);

		ImGui::EndPopup();
	}
}

//--------------------
// RmlDebug::GetScale
//
// return the in scale or a proportional scale smaller or equal than the max scale
//
vec2 RmlDebug::GetScale(vec2 const inScale, vec2 const maxScale) const
{
	vec2 ret = inScale;
	if (ret.x > maxScale.x)
	{
		ret.y *= maxScale.x / ret.x;
		ret.x = maxScale.x;
	}

	if (ret.y > maxScale.y)
	{
		ret.x *= maxScale.y / ret.y;
		ret.y = maxScale.y;
	}

	return ret;
}


} // namespace gui
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)
