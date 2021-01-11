#include "stdafx.h"
#include "TextureFormat.h"
#include <EtCore/Reflection/ReflectionUtil.h>


namespace et {
namespace render {


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::enumeration<E_CompressionSetting>("E_CompressionSetting") (
		value("Default", E_CompressionSetting::Default),
		value("NormalMap", E_CompressionSetting::NormalMap),
		value("GrayScale", E_CompressionSetting::GrayScale),
		value("DisplacementMap", E_CompressionSetting::DisplacementMap),
		value("VectorDisplacementMap", E_CompressionSetting::VectorDisplacementMap),
		value("HDR", E_CompressionSetting::HDR),
		value("UI", E_CompressionSetting::UI),
		value("Alpha", E_CompressionSetting::Alpha),
		value("SdfFont", E_CompressionSetting::SdfFont),
		value("BC7", E_CompressionSetting::BC7));

	registration::enumeration<E_SrgbSetting>("E_SrgbSetting") (
		value("None", E_SrgbSetting::None),
		value("OnLoad", E_SrgbSetting::OnLoad));
}

DEFINE_FORCED_LINKING(TextureFormatRegistrationLinkEnforcer) // force the linker to include this unit


//-----------------------
// RequiresCompression
//
bool RequiresCompression(E_CompressionSetting const setting)
{
	switch (setting)
	{
	case E_CompressionSetting::Default:
	case E_CompressionSetting::NormalMap:
	case E_CompressionSetting::Alpha:
	case E_CompressionSetting::BC7:
		return true;

	case E_CompressionSetting::GrayScale:
	case E_CompressionSetting::DisplacementMap:
	case E_CompressionSetting::VectorDisplacementMap:
	case E_CompressionSetting::HDR:
	case E_CompressionSetting::UI:
	case E_CompressionSetting::SdfFont:
		return false;
	}

	ET_ASSERT(false, "unhandled compression setting '%s'", core::reflection::EnumString(setting).c_str());
	return false;
}


} // namespace render
} // namespace et
