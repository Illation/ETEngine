#pragma once


namespace et {
namespace core {

namespace XML {


//-----------
// Attribute
//
// Specify properties of an element
//
struct Attribute
{
	core::HashString m_Name;
	std::string m_Value;
};

//---------
// Element
//
// Contains full XML structure including file node and root element
//
struct Element
{
	Attribute const* GetAttribute(T_Hash const id) const;
	Element const* GetFirstChild(T_Hash const id) const;
	Element const* GetFirstChild(T_Hash const id, size_t& pos) const;

	core::HashString m_Name;
	std::vector<Attribute> m_Attributes;

	std::vector<Element> m_Children;
	std::string m_Value;
};

//----------
// Document
//
// Contains full XML structure including file node and root element
//
struct Document final
{
	// definitions
	//-------------
	static constexpr char s_UndefinedVersion[] = "undefined";

	// All possible types of encoding for XML documents
	enum class E_Enconding : uint8
	{
		UTF_8,
		UTF_16,

		ISO_10646_UCS_2,
		ISO_10646_UCS_4,

		ISO_8859_1,
		ISO_8859_2,
		ISO_8859_3,
		ISO_8859_4,
		ISO_8859_5,
		ISO_8859_6,
		ISO_8859_7,
		ISO_8859_8,
		ISO_8859_9,

		ISO_2022_JP,

		Shift_JIS,

		EUC_JP
	};


	// Data
	///////

	std::string m_Version = s_UndefinedVersion;
	E_Enconding m_Encoding = E_Enconding::UTF_8;
	bool m_IsStandalone = false;

	Element m_Root;
};


} // namespace XML

} // namespace core
} // namespace et
