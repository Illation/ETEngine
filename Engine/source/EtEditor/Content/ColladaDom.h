#pragma once


namespace et {
namespace edit {

//--------------------------
//
// not all information is parsed into the DOM, but instead should be parsed on demand depending on import settings
//
namespace dae { // COLLADA


//--------------------------
// Asset
//
// Global information for the collada document
//
struct Asset
{
	enum class E_Axis : uint8
	{
		X,
		Y,
		Z
	};

	XML::Element* m_Element = nullptr;
	float m_UnitToMeter = 1.f;
	E_Axis m_UpAxis = E_Axis::Y;
};

//--------------------------
// Document
//
// Root Collada document
//
struct Document
{
	std::string m_Version;
	Asset m_Asset;
};


} // namespace dae

} // namespace edit
} // namespace et

