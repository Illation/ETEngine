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

//-------------------------------
// ColladaParser::ReadSourceList
//
void ColladaParser::ReadSourceList(std::vector<dae::Source>& sources, core::XML::Element const& parent)
{
	static size_t const s_SourceHash = "source"_hash;
	static size_t const s_AccessorHash = "accessor"_hash;

	size_t pos = 0u;
	core::XML::Element const* sourceEl = parent.GetFirstChild(s_SourceHash, pos);
	while (sourceEl != nullptr)
	{
		core::XML::Attribute const* const idAttrib = sourceEl->GetAttribute("id"_hash);
		if (idAttrib == nullptr)
		{
			// next lib - source is inaccessible so it is of no use to us
			sourceEl = parent.GetFirstChild(s_SourceHash, ++pos);
			continue;
		}

		if (sourceEl->m_Children.size() < 1u)
		{
			// next lib - no source data / not enough room for source + common
			sourceEl = parent.GetFirstChild(s_SourceHash, ++pos);
			continue;
		}

		size_t elIdx = 0u;
		if (sourceEl->m_Children[0u].m_Name == "asset"_hash)
		{
			LOG("Collada data source has asset override, which is currently not supported and may cause problems", core::Warning);
			elIdx++;

			if (sourceEl->m_Children.size() < 2u)
			{
				// next lib - no source data + room for common accessor
				sourceEl = parent.GetFirstChild(s_SourceHash, ++pos);
				continue;
			}
		}

		// find source and accessor elements
		dae::Source::E_Type sourceType;
		core::XML::Element const* dataEl = nullptr;
		core::XML::Element const* accessorEl = nullptr;
		switch (sourceEl->m_Children[elIdx].m_Name.Get())
		{
		case "IDREF_array"_hash:
			dataEl = &sourceEl->m_Children[elIdx];
			sourceType = dae::Source::E_Type::IDREF;
			break;

		case "Name_array"_hash:
			dataEl = &sourceEl->m_Children[elIdx];
			sourceType = dae::Source::E_Type::Name;
			break;

		case "bool_array"_hash:
			dataEl = &sourceEl->m_Children[elIdx];
			sourceType = dae::Source::E_Type::Bool;
			break;

		case "float_array"_hash:
			dataEl = &sourceEl->m_Children[elIdx];
			sourceType = dae::Source::E_Type::Float;
			break;

		case "int_array"_hash:
			dataEl = &sourceEl->m_Children[elIdx];
			sourceType = dae::Source::E_Type::Int;
			break;

		case "technique_common"_hash:
		{
			sourceType = dae::Source::E_Type::None;
			accessorEl = sourceEl->m_Children[elIdx].GetFirstChild(s_AccessorHash);
		}

		default:
			// next lib - no supported source data
			LOG(FS("Expected data source or accessor, found '%s'.", sourceEl->m_Children[elIdx].m_Name.ToStringDbg()), core::Warning);
			sourceEl = parent.GetFirstChild(s_SourceHash, ++pos);
			continue;
		}

		if (accessorEl == nullptr)
		{
			elIdx++;
			if ((sourceEl->m_Children.size() > elIdx) && (sourceEl->m_Children[elIdx].m_Name == "technique_common"_hash))
			{
				accessorEl = sourceEl->m_Children[elIdx].GetFirstChild(s_AccessorHash);
			}
			else if (dataEl == nullptr)
			{
				// we have neither data nor accessor so this source is useless to us
				sourceEl = parent.GetFirstChild(s_SourceHash, ++pos);
				continue;
			}
		}

		// data ID
		core::HashString dataId;
		if (dataEl != nullptr)
		{
			core::XML::Attribute const* const dataIdAttrib = dataEl->GetAttribute("id"_hash);
			if (dataIdAttrib != nullptr)
			{
				dataId = core::HashString(dataIdAttrib->m_Value.c_str());
			}
		}

		// accessor
		dae::Accessor* accessor = nullptr;
		if (accessorEl != nullptr)
		{
			dae::Accessor temp;
			if (ReadAccessor(temp, *accessorEl))
			{
				accessor = new dae::Accessor(temp);
			}
		}

		// we now know this source has all the info we need and therfore add it to the list
		sources.emplace_back(core::HashString(idAttrib->m_Value.c_str()), dataId, sourceType, accessor, *sourceEl);
				
		// next lib
		sourceEl = parent.GetFirstChild(s_SourceHash, ++pos);
	}
}

//-----------------------------
// ColladaParser::ReadAccessor
//
bool ColladaParser::ReadAccessor(dae::Accessor& accessor, core::XML::Element const& accessorEl)
{
	ET_ASSERT(accessorEl.m_Name == "accessor"_hash);

	// required attributes
	core::XML::Attribute const* const countAttrib = accessorEl.GetAttribute("count"_hash);
	if (countAttrib == nullptr)
	{
		LOG("Invalid accessor found, no count attribute was present");
		return false;
	}

	accessor.m_Count = static_cast<size_t>(std::stoul(countAttrib->m_Value));

	// optional attributes
	core::XML::Attribute const* const offset = accessorEl.GetAttribute("offset"_hash);
	if (offset != nullptr)
	{
		accessor.m_Offset = static_cast<size_t>(std::stoul(offset->m_Value));
	}

	core::XML::Attribute const* const stride = accessorEl.GetAttribute("stride"_hash);
	if (stride != nullptr)
	{
		accessor.m_Stride = static_cast<size_t>(std::stoul(stride->m_Value));
	}

	core::XML::Attribute const* const source = accessorEl.GetAttribute("source"_hash);
	if (source != nullptr)
	{
		if (!(source->m_Value.empty()) && source->m_Value[0] == '#')
		{
			accessor.m_SourceDataId = core::HashString(source->m_Value.substr(1u).c_str());
		}
		else
		{
			LOG(FS("Expected accessor source attribute to be a URI fragment starting with #, source: '%s'", source->m_Value.c_str()), core::Warning);
		}
	}

	// children
	static size_t const s_ParamHash = "param"_hash;

	size_t pos = 0u;
	core::XML::Element const* paramEl = accessorEl.GetFirstChild(s_ParamHash, pos);
	while (paramEl != nullptr)
	{
		dae::Accessor::Param param;

		core::XML::Attribute const* const name = paramEl->GetAttribute("name"_hash);
		if (name != nullptr)
		{
			param.m_IsNamed = !(name->m_Value.empty());
		}

		core::XML::Attribute const* const type = paramEl->GetAttribute("type"_hash);
		if (type == nullptr)
		{
			LOG("Accessor has invalid parameter without type, skipping parameter", core::Warning);
			paramEl = accessorEl.GetFirstChild(s_ParamHash, ++pos);
			continue;
		}

		switch (GetHash(type->m_Value))
		{
		case "name"_hash:
			param.m_Type = dae::Accessor::E_ParamType::Name;
			break;

		case "float"_hash:
			param.m_Type = dae::Accessor::E_ParamType::Float;
			break;

		case "float4x4"_hash:
			param.m_Type = dae::Accessor::E_ParamType::Float4x4;
			break;

		default:
			LOG(FS("Unrecognized parameter type '%s', skipping parameter", type->m_Value.c_str()), core::Warning);
			paramEl = accessorEl.GetFirstChild(s_ParamHash, ++pos);
			continue;
		}

		accessor.m_Parameters.push_back(param);

		// next param
		paramEl = accessorEl.GetFirstChild(s_ParamHash, ++pos);
	}

	return true;
}

//------------------------------
// ColladaParser::ReadInputList
//
void ColladaParser::ReadInputList(std::vector<dae::Input>& inputs, core::XML::Element const& parent, bool const isShared)
{
	static size_t const s_InputHash = "input"_hash;

	size_t pos = 0u;
	core::XML::Element const* inputEl = parent.GetFirstChild(s_InputHash, pos);
	while (inputEl != nullptr)
	{
		dae::Input input;

		core::XML::Attribute const* const semantic = inputEl->GetAttribute("semantic"_hash);
		if (semantic == nullptr)
		{
			LOG("Input is missing semantic, skipping", core::Warning);
			inputEl = parent.GetFirstChild(s_InputHash, ++pos);
			continue;
		}

		input.m_Semantic = ReadSemantic(semantic->m_Value);
		if (input.m_Semantic == dae::E_Semantic::Invalid)
		{
			LOG(FS("Failed to read semantic '%s', skipping input", semantic->m_Value.c_str()), core::Warning);
			inputEl = parent.GetFirstChild(s_InputHash, ++pos);
			continue;
		}

		// source points to an accessor
		core::XML::Attribute const* const source = inputEl->GetAttribute("source"_hash);
		if (source == nullptr)
		{
			LOG("Input is missing source, skipping", core::Warning);
			inputEl = parent.GetFirstChild(s_InputHash, ++pos);
			continue;
		}
		else if ((source->m_Value.empty()) || (source->m_Value[0] != '#'))
		{
			LOG(FS("Expected input source attribute to be a URI fragment starting with #, source: '%s'", source->m_Value.c_str()), core::Warning);
			inputEl = parent.GetFirstChild(s_InputHash, ++pos);
			continue;
		}

		input.m_Source = core::HashString(source->m_Value.substr(1u).c_str());

		// following attributes don't exist on unshared inputs
		if (isShared)
		{
			core::XML::Attribute const* const offset = inputEl->GetAttribute("offset"_hash);
			if (offset == nullptr)
			{
				LOG("Shared input is missing offset, skipping", core::Warning);
				inputEl = parent.GetFirstChild(s_InputHash, ++pos);
				continue;
			}

			input.m_Offset = static_cast<size_t>(std::stoul(offset->m_Value));

			core::XML::Attribute const* const set = inputEl->GetAttribute("set"_hash);
			if (set != nullptr)
			{
				input.m_Set = static_cast<size_t>(std::stoul(set->m_Value));
			}
		}

		inputs.push_back(input);

		// next lib
		inputEl = parent.GetFirstChild(s_InputHash, ++pos);
	}
}

//-----------------------------
// ColladaParser::ReadSemantic
//
dae::E_Semantic ColladaParser::ReadSemantic(std::string const& semantic)
{
	switch (GetHash(semantic))
	{
	case "BINORMAL"_hash:			return dae::E_Semantic::Binormal;
	case "COLOR"_hash:				return dae::E_Semantic::Color;
	case "CONTINUITY"_hash:			return dae::E_Semantic::Continuity;
	case "IMAGE"_hash:				return dae::E_Semantic::Image;
	case "INPUT"_hash:				return dae::E_Semantic::Input;
	case "IN_TANGENT"_hash:			return dae::E_Semantic::InTangent;
	case "INTERPOLATION"_hash:		return dae::E_Semantic::Interpolation;
	case "INV_BIND_MATRIX"_hash:	return dae::E_Semantic::InvBindMatrix;
	case "JOINT"_hash:				return dae::E_Semantic::Joint;
	case "LINEAR_STEPS"_hash:		return dae::E_Semantic::LinearSteps;
	case "MORPH_TARGET"_hash:		return dae::E_Semantic::MorphTarget;
	case "MORPH_WEIGHT"_hash:		return dae::E_Semantic::MorphWeight;
	case "NORMAL"_hash:				return dae::E_Semantic::Normal;
	case "OUTPUT"_hash:				return dae::E_Semantic::Output;
	case "OUT_TANGENT"_hash:		return dae::E_Semantic::OutTangent;
	case "POSITION"_hash:			return dae::E_Semantic::Position;
	case "TANGENT"_hash:			return dae::E_Semantic::Tangent;
	case "TEXBINORMAL"_hash:		return dae::E_Semantic::TexBiNormal;
	case "TEXCOORD"_hash:			return dae::E_Semantic::Texcoord;
	case "TEXTANGENT"_hash:			return dae::E_Semantic::TexTangent;
	case "UV"_hash:					return dae::E_Semantic::UV;
	case "VERTEX"_hash:				return dae::E_Semantic::Vertex;
	case "WEIGHT"_hash:				return dae::E_Semantic::Weight;
	}

	return dae::E_Semantic::Invalid;
}


} // namespace edit
} // namespace et
