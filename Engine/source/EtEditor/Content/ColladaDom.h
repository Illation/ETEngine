#pragma once
#include <EtCore/IO/XmlDom.h>


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
struct Asset final
{
	enum class E_Axis : uint8
	{
		X,
		Y,
		Z
	};

	core::XML::Element const* m_Element = nullptr; // not owned
	float m_UnitToMeter = 1.f;
	E_Axis m_UpAxis = E_Axis::Y;
};

//--------------------------
// Library
//
// Non specific collection of collada elements
//
struct Library final
{
	~Library() { delete m_Asset; }

	Asset* m_Asset = nullptr; // a library can override the base document asset
	std::vector<core::XML::Element const*> m_Elements;
	// we don't care about extras
};

//--------------------------
// Document
//
// Root Collada document
//
struct Document final
{
	std::string m_Version;
	Asset m_Asset;

	std::vector<Library> m_AnimationLibraries;
	std::vector<Library> m_AnimationClipLibraries;
	std::vector<Library> m_ControllerLibraries;
	std::vector<Library> m_GeometryLibraries;
	std::vector<Library> m_NodeLibraries;
	std::vector<Library> m_VisualSceneLibraries;

	std::string m_VisualSceneUrl; // extracted from instance_visual_scene in scene
};


} // namespace dae

} // namespace edit
} // namespace et

