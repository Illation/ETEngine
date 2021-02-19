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

//----------
// Accessor
//
// Access pattern into a source array
//
struct Accessor
{
	// definitions
	//-------------
	enum class E_ParamType : uint8
	{
		Name,
		Float,
		Float4x4
		// add more as needed
	};

	struct Param
	{
		bool m_IsNamed = false;
		E_ParamType m_Type;
	};

	// Data
	///////
	
	size_t m_Count;
	size_t m_Offset = 0u;
	size_t m_Stride = 1u;

	core::HashString m_SourceDataId;

	std::vector<Param> m_Parameters;
};

//----------
// Source
//
// data buffer
//
struct Source final
{
	// definitions
	//-------------
	enum class E_Type : uint8
	{
		IDREF,
		Name,
		Bool,
		Float,
		Int,

		None
	};

	// construct destruct
	//--------------------
	Source(core::HashString const id, core::HashString const dataId, E_Type const type, Accessor const* const accessor, core::XML::Element const& el);
	Source(Source const& other);
	Source& operator=(Source const& other);
	Source(Source&& other);
	Source& operator=(Source&& other);
	~Source();

	// Data
	///////

	core::HashString m_Id;
	core::HashString m_DataId;

	E_Type m_Type;
	std::vector<uint8> m_Buffer;

	Accessor const* m_CommonAccessor = nullptr;

	core::XML::Element const* m_Element; // not owned
	bool m_IsParsed = false;
};

//------------
// E_Semantic
//
// Standard semantic values in an Input element
//
enum class E_Semantic : uint8
{
	Vertex, // redirect

	Position, // standard mesh components
	Normal,
	Tangent,
	Binormal,
	Color,
	Texcoord,

	TexTangent, // extra texcoord components
	TexBiNormal,

	InvBindMatrix, // skinning
	Joint,
	Weight,

	MorphTarget, // morphing
	MorphWeight,

	InTangent, // animations
	OutTangent,

	Continuity, // other
	Image, 
	Input,
	Interpolation,
	LinearSteps,
	Output,
	UV,

	Invalid
};

//-------
// Input
//
// Link a semantic to a source / accessor
//
struct Input final
{
	static size_t const s_InvalidIndex;

	// all kinds of inputs
	E_Semantic m_Semantic = E_Semantic::Invalid;
	core::HashString m_Source;

	// for shared inputs
	size_t m_Offset = s_InvalidIndex;
	size_t m_Set = s_InvalidIndex;
};



} // namespace dae

} // namespace edit
} // namespace et

