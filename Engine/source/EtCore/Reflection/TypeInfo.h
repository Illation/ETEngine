#pragma once


namespace et {
namespace core {


//------------------
// TypeInfo
//
// Includes rttr type info combined with IDs that are stable for serialization
//
struct TypeInfo final
{
	// definitions
	//-------------
	struct PropertyInfo
	{
		PropertyInfo(rttr::property const prop);

		HashString m_Id;
		rttr::property m_Property;

		bool m_IsSerializable = true;
	};

	enum class E_Kind : uint8
	{
		Arithmetic,
		Enumeration,
		Vector,
		String,
		Hash,
		AssetPointer,
		ContainerSequential,
		ContainerAssociative,
		Class,

		Undefined
	};

	// static utility
	//----------------
	static bool IsBasic(rttr::type const type);

	// functionality
	//---------------
	TypeInfo(rttr::type const type);

	rttr::property const* GetProperty(HashString const id) const;

	// Data
	///////

	HashString m_Id;

	rttr::type m_Type;
	E_Kind m_Kind = E_Kind::Undefined;
	std::vector<PropertyInfo> m_Properties;
};


} // namespace core
} // namespace et
