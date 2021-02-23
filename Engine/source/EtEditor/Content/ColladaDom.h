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
	// definitions
	//-------------
	enum class E_Axis : uint8
	{
		X,
		Y,
		Z
	};

	// accessors for coordinate system conversion
	ivec3 Get3DIndices() const;
	vec3 Get3DAxisMultipliers() const;

	// Data
	///////

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
// Instance
//
struct Instance 
{
	// definitions
	//-------------
private:
	static std::vector<core::HashString> const s_XmlIds; // same order as type

public:
	enum class E_Type : uint8
	{
		VisualScene,
		Node,
		Geometry,

		None
	};

	static E_Type GetTypeFromXmlId(core::HashString const id);
	static core::HashString GetXmlIdFromType(E_Type const type);

	// construct destruct
	//--------------------
	Instance(E_Type const type = E_Type::None) : m_Type(type) {}
	Instance(E_Type const type, core::HashString const url) : m_Type(type), m_Url(url) {}

	// Data
	///////

	E_Type m_Type;
	core::HashString m_Url;
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

	Instance m_VisualSceneInstance; 
};

struct Source;

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

	// functionality
	//---------------
	template <uint8 n>
	math::vector<n, float> ReadVector(Source const& source, size_t const idx) const;

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

	static uint8 GetTypeSize(E_Type const type);

	// construct destruct
	//--------------------
	Source(core::HashString const id, core::HashString const dataId, E_Type const type, Accessor const* const accessor, core::XML::Element const& dataEl);
	Source(Source const& other);
	Source& operator=(Source const& other);
	Source(Source&& other);
	Source& operator=(Source&& other);
	~Source();

	// accessors
	//-----------
	template <typename TDataType>
	TDataType Read(size_t const idx) const; // unsafe, ensure TDataType matches E_Type

	// Data
	///////

	core::HashString m_Id;
	core::HashString m_DataId;

	// asset ignored for now

	E_Type m_Type;
	uint8 m_TypeSize = 0u;
	std::vector<uint8> m_Buffer;

	Accessor const* m_CommonAccessor = nullptr;

	core::XML::Element const* m_DataEl; // not owned
	bool m_IsResolved = false;
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

//---------------
// ResolvedInput
//
struct ResolvedInput
{
	dae::Input m_Input;
	dae::Accessor const* m_Accessor;
	dae::Source* m_Source;
};

//------
// Mesh
//
// currently only supports a single primitive
//
struct Mesh
{
	std::vector<dae::Source> m_Sources;

	std::vector<ResolvedInput> m_ResolvedInputs;
	size_t m_MaxInputOffset = 0u;

	size_t m_FaceCount;
	std::vector<size_t> m_PrimitiveIndices;
	std::vector<uint8> m_VertexCounts; // in case of polylist
};

//------
// Node
//
struct Node
{
	bool GetGeometryTransform(mat4& base, core::HashString const geometryId) const;

	// Data
	///////

	core::HashString m_Id;

	// asset ignored
	
	mat4 m_Transform; // in collada coordinate system
	std::vector<Instance> m_Instances;
	std::vector<Node> m_Children;
};

//-------------
// VisualScene
//
struct VisualScene
{
	core::HashString m_Id;

	// asset ignored

	std::vector<Node> m_Nodes;
};


} // namespace dae

} // namespace edit
} // namespace et


#include "ColladaDom.inl"
