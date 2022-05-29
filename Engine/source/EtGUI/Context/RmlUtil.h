#pragma once
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Types.h>

#include <EtCore/Input/KeyCodes.h>


namespace et {
namespace gui {

namespace RmlUtil {

	int32 GetRmlButtonIndex(E_MouseButton const button);
	Rml::Input::KeyIdentifier GetRmlKeyId(E_KbdKey const key);
	int32 GetRmlModifierFlags(core::T_KeyModifierFlags const mods);


	// Converting et::math types to Rml counterparts
	template <typename TDataType>
	Rml::Vector2<TDataType> ToRml(math::vector<2, TDataType> const& rhs)
	{
		return Rml::Vector2<TDataType>(rhs.x, rhs.y);
	}

	template <typename TDataType>
	Rml::Vector3<TDataType> ToRml(math::vector<3, TDataType> const& rhs)
	{
		return Rml::Vector3<TDataType>(rhs.x, rhs.y, rhs.z);
	}

	template <typename TDataType>
	Rml::Vector4<TDataType> ToRml(math::vector<4, TDataType> const& rhs)
	{
		return Rml::Vector4<TDataType>(rhs.x, rhs.y, rhs.z, rhs.w);
	}

	// Converting et::math types from Rml counterparts
	template <typename TDataType>
	math::vector<2, TDataType> ToEtm(Rml::Vector2<TDataType> const& rhs)
	{
		return math::vector<2, TDataType>(rhs.x, rhs.y);
	}

	template <typename TDataType>
	math::vector<3, TDataType> ToEtm(Rml::Vector3<TDataType> const& rhs)
	{
		return math::vector<3, TDataType>(rhs.x, rhs.y, rhs.z);
	}

	template <typename TDataType>
	math::vector<4, TDataType> ToEtm(Rml::Vector4<TDataType> const& rhs)
	{
		return math::vector<4, TDataType>(rhs.x, rhs.y, rhs.z, rhs.w);
	}

} // namespace RmlUtil

} // namespace gui
} // namespace et

