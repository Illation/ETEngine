#include "stdafx.h"
#include "BasicGuiRenderer.h"

#include <EtCore/Input/RawInputProvider.h>

#include <EtGUI/Context/RmlUtil.h>

#include <EtRHI/GraphicsTypes/TextureData.h>
#include <EtRHI/GraphicsTypes/Shader.h>


namespace et {
namespace app {


//====================
// Basic GUI Renderer
//====================


//-------------------------
// BasicGuiRenderer::d-tor
//
BasicGuiRenderer::~BasicGuiRenderer()
{
	if (m_InputProvider != nullptr)
	{
		Deinit();
	}
}

//------------------------
// BasicGuiRenderer::Init
//
void BasicGuiRenderer::Init(Ptr<core::RawInputProvider> const inputProvider)
{
	m_GuiRenderer.Init();

	ET_ASSERT(inputProvider != nullptr);
	m_InputProvider = inputProvider;
	m_InputProvider->RegisterListener(ToPtr(this));
}

//--------------------------------------------
// BasicGuiRenderer::Deinit
//
// Remove references to textures etc.
//
void BasicGuiRenderer::Deinit()
{
	m_InputProvider->UnregisterListener(this);
	m_InputProvider = nullptr;

	m_GuiRenderer.Deinit();
	m_ContextRenderTarget.DeleteFramebuffer();
	m_GuiContext.Deinit();
}

//----------------------------------
// BasicGuiRenderer::SetGuiDocument
//
void BasicGuiRenderer::SetGuiDocument(core::HashString const documentId)
{
	if (documentId.IsEmpty())
	{
		if (m_GuiContext.GetDocumentCount() > 0u)
		{
			m_GuiContext.UnloadDocument(m_GuiContext.GetDocumentId(0u));
			m_GuiContext.Deinit();
		}
	}
	else if (m_GuiContext.GetDocumentCount() == 0u)
	{
		m_GuiContext.Init("Basic GUI Context", rhi::Viewport::GetCurrentViewport()->GetDimensions());
		m_GuiContext.LoadDocument(documentId);
	}
}

//----------------------------
// BasicGuiRenderer::OnResize
//
void BasicGuiRenderer::OnResize(ivec2 const dim)
{
	if (m_InputProvider != nullptr && m_GuiContext.HasActiveDocuments())
	{
		m_GuiContext.SetDimensions(dim);
	}
}

//----------------------------
// BasicGuiRenderer::OnRender
//
// Main scene drawing function
//
void BasicGuiRenderer::OnRender(rhi::T_FbLoc const targetFb)
{
	if ((m_InputProvider == nullptr) || !m_GuiContext.HasActiveDocuments())
	{
		return;
	}

	m_GuiContext.Update();

	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();
	device->SetClearColor(vec4(0.f, 0.f, 0.f, 0.f));
	device->Clear(rhi::E_ClearFlag::CF_Color);

	rhi::Viewport const* const viewport = rhi::Viewport::GetCurrentViewport();
	m_ContextRenderTarget.UpdateForDimensions(viewport->GetDimensions());
	m_GuiRenderer.RenderContext(targetFb, m_ContextRenderTarget, m_GuiContext);
}


//--------------------------------
// BasicGuiRenderer::ProcessKeyPressed
//
bool BasicGuiRenderer::ProcessKeyPressed(E_KbdKey const key, core::T_KeyModifierFlags const modifiers)
{
	Rml::Input::KeyIdentifier const rmlKey = gui::RmlUtil::GetRmlKeyId(key);
	if (rmlKey != Rml::Input::KeyIdentifier::KI_UNKNOWN)
	{
		int32 const mods = gui::RmlUtil::GetRmlModifierFlags(modifiers);
		if (m_GuiContext.HasActiveDocuments())
		{
			return m_GuiContext.ProcessKeyPressed(rmlKey, mods);
		}
	}

	return false;
}

//---------------------------------
// BasicGuiRenderer::ProcessKeyReleased
//
bool BasicGuiRenderer::ProcessKeyReleased(E_KbdKey const key, core::T_KeyModifierFlags const modifiers)
{
	Rml::Input::KeyIdentifier const rmlKey = gui::RmlUtil::GetRmlKeyId(key);
	if (rmlKey != Rml::Input::KeyIdentifier::KI_UNKNOWN)
	{
		int32 const mods = gui::RmlUtil::GetRmlModifierFlags(modifiers);
		if (m_GuiContext.HasActiveDocuments())
		{
			return m_GuiContext.ProcessKeyReleased(rmlKey, mods);
		}
	}

	return false;
}

//----------------------------------
// BasicGuiRenderer::ProcessMousePressed
//
bool BasicGuiRenderer::ProcessMousePressed(E_MouseButton const button, core::T_KeyModifierFlags const modifiers)
{
	int32 const rmlButton = gui::RmlUtil::GetRmlButtonIndex(button);
	if (rmlButton != -1)
	{
		int32 const mods = gui::RmlUtil::GetRmlModifierFlags(modifiers);
		if (m_GuiContext.HasActiveDocuments())
		{
			return m_GuiContext.ProcessMousePressed(rmlButton, mods);
		}
	}

	return false;
}

//-----------------------------------
// BasicGuiRenderer::ProcessMouseReleased
//
bool BasicGuiRenderer::ProcessMouseReleased(E_MouseButton const button, core::T_KeyModifierFlags const modifiers)
{
	int32 const rmlButton = gui::RmlUtil::GetRmlButtonIndex(button);
	if (rmlButton != -1)
	{
		int32 const mods = gui::RmlUtil::GetRmlModifierFlags(modifiers);
		if (m_GuiContext.HasActiveDocuments())
		{
			return m_GuiContext.ProcessMouseReleased(rmlButton, mods);
		}
	}

	return false;
}

//-------------------------------
// BasicGuiRenderer::ProcessMouseMove
//
bool BasicGuiRenderer::ProcessMouseMove(ivec2 const& mousePos, core::T_KeyModifierFlags const modifiers)
{
	int32 const mods = gui::RmlUtil::GetRmlModifierFlags(modifiers);
	if (m_GuiContext.HasActiveDocuments())
	{
		return m_GuiContext.ProcessMouseMove(mousePos, mods);
	}

	return false;
}

//-------------------------------------
// BasicGuiRenderer::ProcessMouseWheelDelta
//
bool BasicGuiRenderer::ProcessMouseWheelDelta(ivec2 const& mouseWheel, core::T_KeyModifierFlags const modifiers)
{
	ivec2 const delta(mouseWheel.x, -mouseWheel.y);
	int32 const mods = gui::RmlUtil::GetRmlModifierFlags(modifiers);
	if (m_GuiContext.HasActiveDocuments())
	{
		return m_GuiContext.ProcessMouseWheelDelta(delta, mods);
	}

	return false;
}

//-------------------------------------
// BasicGuiRenderer::ProcessMouseEnterLeave
//
bool BasicGuiRenderer::ProcessMouseEnterLeave(bool const entered, core::T_KeyModifierFlags const)
{
	if (entered)
	{
		return false;
	}

	if (m_GuiContext.HasActiveDocuments())
	{
		m_GuiContext.ProcessMouseLeave();
	}

	return false;
}

//---------------------------------
// BasicGuiRenderer::ProcessTextInput
//
bool BasicGuiRenderer::ProcessTextInput(core::E_Character const character)
{
	if (m_GuiContext.HasActiveDocuments())
	{
		return m_GuiContext.ProcessTextInput(static_cast<Rml::Character>(character));
	}

	return false;
}


} // namespace app
} // namespace et

