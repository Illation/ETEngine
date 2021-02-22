#pragma once
#include <EtCore/IO/XmlParser.h>

#include "ColladaDom.h"


namespace et {
namespace edit {


//----------------
// ColladaParser
//
// The goal is not to be fully compliant, but to read the necessary information as fast as possible
//
class ColladaParser final
{
	// definitions
	//-------------
	static float const s_MinVersion;
	static float const s_MaxVersion;
public:
	typedef std::function<void(core::XML::Element const& el, dae::Asset const& asset)> T_PerElementFn;

	// construct destruct
	//--------------------
	ColladaParser(std::vector<uint8> const& colladaData);

	// accessors
	//-----------
	bool IsValid() const { return m_IsValid; }
	
	inline void IterateAnimations(T_PerElementFn const& perElFn) const { IterateLibraries(m_Document.m_AnimationLibraries, perElFn); }
	inline void IterateAnimationClips(T_PerElementFn const& perElFn) const { IterateLibraries(m_Document.m_AnimationClipLibraries, perElFn); }
	inline void IterateControllers(T_PerElementFn const& perElFn) const { IterateLibraries(m_Document.m_ControllerLibraries, perElFn); }
	inline void IterateGeometries(T_PerElementFn const& perElFn) const { IterateLibraries(m_Document.m_GeometryLibraries, perElFn); }
	inline void IterateNodes(T_PerElementFn const& perElFn) const { IterateLibraries(m_Document.m_NodeLibraries, perElFn); }
	inline void IterateVisualScenes(T_PerElementFn const& perElFn) const { IterateLibraries(m_Document.m_VisualSceneLibraries, perElFn); }

	// static functionality
	//----------------------
	static std::string const& GetLibraryElementName(core::XML::Element const& libraryEl);
	static size_t GetPrimitiveCount(core::XML::Element const& meshEl);
	static void ReadSourceList(std::vector<dae::Source>& sources, core::XML::Element const& parent);
	static bool ResolveSource(dae::Source& source);
	static bool ReadAccessor(dae::Accessor& accessor, core::XML::Element const& accessorEl);
	static void ReadInputList(std::vector<dae::Input>& inputs, core::XML::Element const& parent, bool const isShared);
	static dae::E_Semantic ReadSemantic(std::string const& semantic);
	static core::XML::Element const* GetMeshElFromGeometry(core::XML::Element const& geometryEl);
	static bool ReadMesh(dae::Mesh& mesh, core::XML::Element const& meshEl);

	template <typename TDataType>
	static void ParseArray(std::vector<TDataType>& vec, core::XML::Element const& el);
	static void ParseArrayU8(std::vector<uint8>& vec, core::XML::Element const& el);

	// utility
	//---------
private:
	core::XML::Element const* GetAsset(core::XML::Element const& parent) const;
	void ReadAsset(dae::Asset& asset, core::XML::Element const& assetEl);
	void ReadLibraries(std::vector<dae::Library>& libraries, core::XML::Element const& parent, T_Hash const libraryName, T_Hash const elementName);

	void IterateLibraries(std::vector<dae::Library> const& libs, T_PerElementFn const& perElFn) const;


	// Data
	///////

	core::XML::Parser const m_Xml;
	dae::Document m_Document;

	bool m_IsValid = true;
};


} // namespace edit
} // namespace et


#include "ColladaParser.inl"
