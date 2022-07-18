#include "stdafx.h"
#include "RmlSystemInterface.h"


namespace et {
namespace gui {


ET_DEFINE_TRACE_CTX(ET_CTX_RMLUI);
ET_REGISTER_TRACE_CTX(ET_CTX_RMLUI);
	

//======================
// RML System Interface
//======================


//------------------------------------
// RmlSystemInterface::GetElapsedTime
//
// Seconds since start of the program
//
double RmlSystemInterface::GetElapsedTime()
{
	return core::ContextManager::GetInstance()->GetActiveContext()->time->GetTime<double>();
}

//---------------------------------
// RmlSystemInterface::LogMessage
//
bool RmlSystemInterface::LogMessage(Rml::Log::Type type, Rml::String const& message)
{
	core::E_TraceLevel level = core::E_TraceLevel::TL_Info;
	bool trace = true;
	bool ret = true;
	switch (type)
	{
	case Rml::Log::Type::LT_ALWAYS:
		trace = false;
	case Rml::Log::Type::LT_INFO:
	default:
		level = core::E_TraceLevel::TL_Info;
		break;

	case Rml::Log::Type::LT_WARNING:
		level = core::E_TraceLevel::TL_Warning;
		break;

	case Rml::Log::Type::LT_ERROR:
		level = core::E_TraceLevel::TL_Error;
		ret = false;
		break;

	case Rml::Log::Type::LT_ASSERT:
		level = core::E_TraceLevel::TL_Warning;
#if ET_CT_IS_ENABLED(ET_CT_ASSERT)
		ret = false;
#endif
		break;

	case Rml::Log::Type::LT_DEBUG:
		level = core::E_TraceLevel::TL_Verbose;
		break;
	}

	if (trace)
	{
		ET_TRACE(ET_CTX_RMLUI, level, false, message.c_str());
	}
	else
	{
		ET_LOG(ET_CTX_RMLUI, level, false, message.c_str());
	}

	return ret;
}

//------------------------------------
// RmlSystemInterface::SetMouseCursor
//
void RmlSystemInterface::SetMouseCursor(Rml::String const& cursor_name)
{
	if (m_CursorShapeManager != nullptr)
	{
		core::HashString const cursorId(cursor_name.c_str());
		switch (cursorId.Get())
		{
		case "text"_hash:
		case "vertical-text"_hash:
			m_CursorShapeManager->SetCursorShape(core::E_CursorShape::IBeam);
			break;

		case "crosshair"_hash:
			m_CursorShapeManager->SetCursorShape(core::E_CursorShape::Crosshair);
			break;

		case "grab"_hash:
		case "grabbing"_hash:
			m_CursorShapeManager->SetCursorShape(core::E_CursorShape::Hand);
			break;

		case "ew-resize"_hash:
		case "e-resize"_hash:
		case "w-resize"_hash:
		case "col-resize"_hash:
			m_CursorShapeManager->SetCursorShape(core::E_CursorShape::SizeWE);
			break;

		case "ns-resize"_hash:
		case "n-resize"_hash:
		case "s-resize"_hash:
		case "row-resize"_hash:
			m_CursorShapeManager->SetCursorShape(core::E_CursorShape::SizeNS);
			break;

		case "none"_hash:
			m_CursorShapeManager->SetCursorShape(core::E_CursorShape::None);
			break;

		case "pointer"_hash:
		default:
			m_CursorShapeManager->SetCursorShape(core::E_CursorShape::Arrow);
			break;
		}
	}
}

//--------------------------------------
// RmlSystemInterface::SetClipboardText
//
void RmlSystemInterface::SetClipboardText(Rml::String const& text)
{
	if (m_ClipboardController != nullptr)
	{
		m_ClipboardController->SetClipboardText(text);
	}
}

//--------------------------------------
// RmlSystemInterface::GetClipboardText
//
void RmlSystemInterface::GetClipboardText(Rml::String& outText)
{
	if (m_ClipboardController != nullptr)
	{
		m_ClipboardController->GetClipboardText(outText);
	}
}


} // namespace gui
} // namespace et
