#include "stdafx.h"
#include "RmlDebug.h"

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)

#include "TickOrder.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

#if ET_CT_IS_ENABLED(ET_CT_RML_DEBUGGER)
#	include <RmlUi/Debugger.h>
#endif

#include <EtCore/Util/DebugCommandController.h>

#include <EtGUI/Rendering/RmlRenderer.h>
#include <EtGUI/Fonts/FontEngine.h>


namespace et {
namespace gui {


//============
// RML Debug
//============


// static
vec2 const RmlDebug::s_MaxTextureScale(150.f);


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

#if ET_CT_IS_ENABLED(ET_CT_RML_DEBUGGER)
//----------------------------------
// RmlDebug::OnContextDestroyed
//
void RmlDebug::OnContextDestroyed(Rml::Context const* const context)
{
	if (m_DebuggerContext.Get() == context)
	{
		Rml::Debugger::Shutdown();
		m_DebuggerContext = nullptr;
		m_DebugContextIdx = -1;
	}
}
#endif

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
			ShowFontUI();
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
#if ET_CT_IS_ENABLED(ET_CT_RML_DEBUGGER)
		if (ImGui::RadioButton("Hide debugger", &m_DebugContextIdx, -1))
		{
			if (m_DebuggerContext != nullptr)
			{
				Rml::Debugger::SetVisible(false);
			}
		}
#else
		ImGui::Text("Rml Debugger disabled - run in debug mode to use");
#endif

		for (int32 ctxIdx = 0; ctxIdx < numContexts; ++ctxIdx)
		{
			Rml::Context* const context = Rml::GetContext(ctxIdx);
			std::string const& contextName = FS("[%i] - %s", ctxIdx, context->GetName().c_str());

#if ET_CT_IS_ENABLED(ET_CT_RML_DEBUGGER)
			if (ImGui::RadioButton(contextName.c_str(), &m_DebugContextIdx, ctxIdx))
			{
				if (m_DebuggerContext == nullptr)
				{
					Rml::Debugger::Initialise(context);
					m_DebuggerContext = ToPtr(context);
				}
				else if (m_DebuggerContext.Get() != context)
				{
					m_DebuggerContext = ToPtr(context);
					Rml::Debugger::SetContext(context);
				}

				Rml::Debugger::SetVisible(true);
			}
#else
			ImGui::Text(contextName.c_str());
#endif
		}
	}
}

//---------------------------
// RmlDebug::ShowRenderingUI
//
void RmlDebug::ShowRenderingUI()
{
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

//----------------------
// RmlDebug::ShowFontUI
//
void RmlDebug::ShowFontUI()
{
	ImGui::SetNextItemOpen(true);
	if (ImGui::CollapsingHeader("Fonts"))
	{
		if (ImGui::TreeNode(FS("Families: " ET_FMT_SIZET, m_FontEngine->m_Families.size()).c_str()))
		{
			for (auto const& familyPair : m_FontEngine->m_Families)
			{
				FontEngine::FontFamily const& family = familyPair.second;
				if (ImGui::TreeNode(family.m_Name.c_str()))
				{
					if (ImGui::TreeNode(FS("Assets: " ET_FMT_SIZET, family.m_UniqueAssets.size()).c_str()))
					{
						for (AssetPtr<SdfFont> const& asset : family.m_UniqueAssets)
						{
							AddFontAsset(asset);
						}

						ImGui::TreePop();
					}

					if (ImGui::TreeNode(FS("Faces: " ET_FMT_SIZET, family.m_FaceIndices.size()).c_str()))
					{
						for (size_t const faceIdx : family.m_FaceIndices)
						{
							AddFontFace(faceIdx);
						}

						ImGui::TreePop();
					}

					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}

		ImGui::Separator();

		if (ImGui::TreeNode(FS("Fallback fonts: " ET_FMT_SIZET, m_FontEngine->m_FallbackFonts.size()).c_str()))
		{
			for (FontEngine::FallbackFont const& fallbackFont : m_FontEngine->m_FallbackFonts)
			{
				if (ImGui::TreeNode(FS("%s - " ET_FMT_SIZET, fallbackFont.m_FamilyId.ToStringDbg(), fallbackFont.m_AssetIdx).c_str()))
				{
					auto const famIt = m_FontEngine->m_Families.find(fallbackFont.m_FamilyId);
					ET_ASSERT(famIt != m_FontEngine->m_Families.cend());
					FontEngine::FontFamily const& family = famIt->second;

					ET_ASSERT(fallbackFont.m_AssetIdx < family.m_UniqueAssets.size());
					AddFontAsset(family.m_UniqueAssets[fallbackFont.m_AssetIdx]);

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
	rhi::ContextHolder::GetRenderDevice()->SetBlendEnabled(TEnabled);
}

//---------------------------
// RmlDebug::AddOpaqueImage
//
// temporarily disable blending to make sure the images are not drawn as transparent
//
void RmlDebug::AddOpaqueImage(rhi::TextureData const* const texture, vec2 const maxScale)
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

//------------------------
// RmlDebug::AddFontAsset
//
void RmlDebug::AddFontAsset(AssetPtr<SdfFont> const& asset)
{
	if (ImGui::TreeNode(asset.GetId().ToStringDbg()))
	{
		ImGui::Text("Family: %s", asset->GetFamily().c_str());
		ImGui::Text("Size: %i", static_cast<int32>(asset->GetFontSize()));
		ImGui::Text("Is italic: %s", asset->IsItalic() ? "true" : "false");
		ImGui::Text("Weight: %i", static_cast<int32>(asset->GetWeight()));
		ImGui::Text("Uses Kerning: %s", asset->UseKerning() ? "true" : "false");
		ImGui::Separator();

		ImGui::Text("Line height: %u", static_cast<uint32>(asset->GetLineHeight()));
		ImGui::Text("Base line: %u", static_cast<uint32>(asset->GetBaseline()));
		ImGui::Text("Underline: %i", static_cast<int32>(asset->GetUnderline()));
		ImGui::Text("Underline thickness: %f", asset->GetUnderlineThickness());

		ImGui::Separator();
		ImGui::Text("SDF size: %f", asset->GetSdfSize());
		AddOpaqueImage(asset->GetAtlas(), s_MaxTextureScale);

		ImGui::TreePop();
	}
}

//------------------------
// RmlDebug::AddFontAsset
//
void RmlDebug::AddFontFace(size_t const faceIdx)
{
	ET_ASSERT(faceIdx < m_FontEngine->m_Faces.size());
	FontEngine::FontFace const& face = m_FontEngine->m_Faces[faceIdx];

	if (ImGui::TreeNode(FS("["ET_FMT_SIZET"]", faceIdx).c_str()))
	{
		switch (face.m_Style)
		{
		case Rml::Style::FontStyle::Italic:
			ImGui::TextUnformatted("Style: Italic");
			break;

		case Rml::Style::FontStyle::Normal:
			ImGui::TextUnformatted("Style: Normal");
			break;
		}

		ImGui::Text("Weight: %i", static_cast<int32>(face.m_Weight));
		ImGui::Text("Size: %i", face.m_Size);

		ImGui::Separator();
		AddFontAsset(face.m_Font);
		ImGui::Separator();

		ImGui::Text("Multiplier: %i", face.m_Multiplier);

		ImGui::TreePop();
	}
}


} // namespace gui
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)
