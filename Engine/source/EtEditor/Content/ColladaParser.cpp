#include "stdafx.h"
#include "ColladaParser.h"

#include <EtCore/FileSystem/FileUtil.h>


namespace et {
namespace edit {


//================
// COLLADA Parser
//================


// static
float const ColladaParser::s_MinVersion = 1.4f;
float const ColladaParser::s_MaxVersion = 1.5f;


//----------------------
// ColladaParser::c-tor
//
// Parse the XML file, and some basic information about the document
//
ColladaParser::ColladaParser(std::vector<uint8> const& colladaData)
	: m_Xml(core::FileUtil::AsText(colladaData))
{
	// start root
	core::XML::Element const& root = m_Xml.GetDocument().m_Root;
	if (root.m_Name != "COLLADA"_hash)
	{
		LOG("Root element was not COLLADA, aborting", core::Warning);
		m_IsValid = false;
		return;
	}

	core::XML::Attribute const* const versionAttrib = root.GetAttribute("version"_hash);
	if (versionAttrib != nullptr)
	{
		m_Document.m_Version = versionAttrib->m_Value;
		float const version = std::stof(m_Document.m_Version);

		if (version < s_MinVersion)
		{
			LOG(FS("Collada document version was smaller than min, assets might be parsed incorrectly. Found '%s', supported '%f'",
				m_Document.m_Version.c_str(),
				s_MinVersion), 
				core::Warning);
		}

		if (version > s_MaxVersion)
		{
			LOG(FS("Collada document version was greater than max, assets might be parsed incorrectly. Found '%s', supported '%f'",
				m_Document.m_Version.c_str(),
				s_MaxVersion),
				core::Warning);
		}
	}
	else
	{
		LOG("Expected COLLADA document to have version attribute in root element", core::Warning);
		m_IsValid = false;
		return;
	}

	// asset
	core::XML::Element const* const assetEl = GetAsset(root);
	if (assetEl != nullptr)
	{
		ReadAsset(m_Document.m_Asset, *assetEl);
	}

	// libraries
	ReadLibraries(m_Document.m_AnimationLibraries, root, "library_animations"_hash, "animation"_hash);
	ReadLibraries(m_Document.m_AnimationClipLibraries, root, "library_animation_clips"_hash, "animation_clip"_hash);
	ReadLibraries(m_Document.m_ControllerLibraries, root, "library_controllers"_hash, "controller"_hash);
	ReadLibraries(m_Document.m_GeometryLibraries, root, "library_geometries"_hash, "geometry"_hash);
	ReadLibraries(m_Document.m_NodeLibraries, root, "library_nodes"_hash, "node"_hash);
	ReadLibraries(m_Document.m_VisualSceneLibraries, root, "library_visual_scenes"_hash, "visual_scene"_hash);

	// scene
	core::XML::Element const* const sceneEl = root.GetFirstChild("scene"_hash);
	if (sceneEl != nullptr)
	{
		core::XML::Element const* const visualSceneEl = sceneEl->GetFirstChild("instance_visual_scene"_hash);
		if (visualSceneEl != nullptr)
		{
			core::XML::Attribute const* const urlAttrib = visualSceneEl->GetAttribute("url"_hash);
			if (urlAttrib != nullptr)
			{
				m_Document.m_VisualSceneUrl = urlAttrib->m_Value;
			}
			else
			{
				LOG("instance_visual_scene was present in collada document but didn't have a URL", core::Warning);
				m_IsValid = false;
				return;
			}
		}
	}
}

//-------------------------
// ColladaParser::GetAsset
//
// return the asset element from a parent, if present, else null
//
core::XML::Element const* ColladaParser::GetAsset(core::XML::Element const& parent) const
{
	return parent.GetFirstChild("asset"_hash);
}

//--------------------------
// ColladaParser::ReadAsset
//
// we ignore most of the asset content that is not strictly required for import
//
void ColladaParser::ReadAsset(dae::Asset& asset, core::XML::Element const& assetEl)
{
	asset.m_Element = &assetEl;

	// we pre-flip unit conversion so we can multiply instead of divide
	core::XML::Element const* const unitEl = assetEl.GetFirstChild("unit"_hash);
	if (unitEl != nullptr)
	{
		core::XML::Attribute const* const meterAttrib = unitEl->GetAttribute("meter"_hash);
		if (meterAttrib != nullptr)
		{
			float const meter = std::stof(meterAttrib->m_Value);
			asset.m_UnitToMeter = 1.f / meter;
		}
	}

	// parse up axis
	core::XML::Element const* const upEl = assetEl.GetFirstChild("up_axis"_hash);
	if (upEl != nullptr)
	{
		switch (GetHash(upEl->m_Value))
		{
		case "X_UP"_hash:
			asset.m_UpAxis = dae::Asset::E_Axis::X;
			break;

		case "Y_UP"_hash:
			asset.m_UpAxis = dae::Asset::E_Axis::Y;
			break;

		case "Z_UP"_hash:
			asset.m_UpAxis = dae::Asset::E_Axis::Z;
			break;

		default:
			LOG(FS("unrecognized up axis '%s' in collada asset", upEl->m_Value.c_str()), core::Warning);
			m_IsValid = false;
			break;
		}
	}
}

//------------------------------
// ColladaParser::ReadLibraries
//
// read abstract libraries
//
void ColladaParser::ReadLibraries(std::vector<dae::Library>& libraries, 
	core::XML::Element const& parent, 
	T_Hash const libraryName, 
	T_Hash const elementName)
{
	size_t pos = 0u;
	core::XML::Element const* libraryEl = parent.GetFirstChild(libraryName, pos);
	while (libraryEl != nullptr)
	{
		libraries.resize(libraries.size() + 1u); // we add to vector first so we don't copy the asset and reallocate memory
		dae::Library& lib = libraries[libraries.size() - 1u];

		// read the libraries asset, if present
		core::XML::Element const* const assetEl = GetAsset(*libraryEl);
		if (assetEl != nullptr)
		{
			lib.m_Asset = new dae::Asset();
			ReadAsset(*lib.m_Asset, *assetEl);
		}

		// read the libraries elements
		size_t elPos = 0u;
		core::XML::Element const* el = libraryEl->GetFirstChild(elementName, elPos);
		while (el != nullptr)
		{
			lib.m_Elements.push_back(el);
			el = libraryEl->GetFirstChild(elementName, ++elPos);
		}

		if (lib.m_Elements.empty())
		{
			LOG(FS("Expected library to have at least one element (type '%s')", core::HashString(elementName).ToStringDbg()), core::Warning);
			m_IsValid = false;
			return;
		}

		// next lib
		libraryEl = parent.GetFirstChild(libraryName, ++pos);
	}
}

//---------------------------------
// ColladaParser::IterateLibraries
//
void ColladaParser::IterateLibraries(std::vector<dae::Library> const& libs, T_PerElementFn const& perElFn) const
{
	for (dae::Library const& lib : libs)
	{
		dae::Asset const& asset = (lib.m_Asset != nullptr) ? *lib.m_Asset : m_Document.m_Asset;
		for (core::XML::Element const* const el : lib.m_Elements)
		{
			perElFn(*el, asset);
		}
	}
}

//---------------------------------
// ColladaParser::IterateLibraries
//
size_t ColladaParser::GetPrimitiveCount(core::XML::Element const& meshEl)
{
	static std::vector<T_Hash> const s_PrimitiveTypes({ "lines"_hash,
		"linestrips"_hash,
		"polygons"_hash,
		"polylist"_hash,
		"triangles"_hash,
		"trifans"_hash,
		"tristrips"_hash });

	ET_ASSERT(meshEl.m_Name == "mesh"_hash);

	size_t count = 0u;
	for (core::XML::Element const& childEl : meshEl.m_Children)
	{
		if (std::find(s_PrimitiveTypes.cbegin(), s_PrimitiveTypes.cend(), childEl.m_Name.Get()) != s_PrimitiveTypes.cend())
		{
			++count;
		}
	}

	return count;
}


} // namespace edit
} // namespace et
