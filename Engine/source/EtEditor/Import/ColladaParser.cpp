#include <EtEditor/stdafx.h>
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
		ET_LOG_W(ET_CTX_EDITOR, "Root element was not COLLADA, aborting");
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
			ET_LOG_W(ET_CTX_EDITOR, 
				"Collada document version was smaller than min, assets might be parsed incorrectly. Found '%s', supported '%f'",
				m_Document.m_Version.c_str(),
				s_MinVersion);
		}

		if (version > s_MaxVersion)
		{
			ET_LOG_W(ET_CTX_EDITOR, 
				"Collada document version was greater than max, assets might be parsed incorrectly. Found '%s', supported '%f'",
				m_Document.m_Version.c_str(),
				s_MaxVersion);
		}
	}
	else
	{
		ET_LOG_W(ET_CTX_EDITOR, "Expected COLLADA document to have version attribute in root element");
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
		core::XML::Element const* const visualSceneEl = sceneEl->GetFirstChild(dae::Instance::GetXmlIdFromType(dae::Instance::E_Type::VisualScene).Get());
		if (visualSceneEl != nullptr)
		{
			m_Document.m_VisualSceneInstance.m_Url = GetInstanceUrl(*visualSceneEl);
			if (!m_Document.m_VisualSceneInstance.m_Url.IsEmpty())
			{
				m_Document.m_VisualSceneInstance.m_Type = dae::Instance::E_Type::VisualScene;
			}
			else
			{
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
			ET_LOG_W(ET_CTX_EDITOR, "unrecognized up axis '%s' in collada asset", upEl->m_Value.c_str());
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
			ET_LOG_W(ET_CTX_EDITOR, "Expected library to have at least one element (type '%s')", core::HashString(elementName).ToStringDbg());
			libraries.pop_back();
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

//-------------------------------
// ColladaParser::GetInstanceUrl
//
core::HashString ColladaParser::GetInstanceUrl(core::XML::Element const& instanceEl)
{
	core::XML::Attribute const* attrib = instanceEl.GetAttribute("url"_hash);
	if (attrib != nullptr)
	{
		if ((attrib->m_Value.size() > 1u) && (attrib->m_Value[0] == '#'))
		{
			return core::HashString(attrib->m_Value.substr(1u).c_str());
		}

		ET_LOG_W(ET_CTX_EDITOR, "Instance '%s' didn't have a fragment type URL", instanceEl.m_Name.ToStringDbg());
	}
	else
	{
		ET_LOG_W(ET_CTX_EDITOR, "Instance '%s' didn't have a URL", instanceEl.m_Name.ToStringDbg());
	}

	return core::HashString();
}

//-----------------------------
// ColladaParser::GetElementId
//
core::HashString ColladaParser::GetElementId(core::XML::Element const& libraryEl)
{
	core::XML::Attribute const* attrib = libraryEl.GetAttribute("id"_hash);
	if (attrib != nullptr)
	{
		return core::HashString(attrib->m_Value.c_str());
	}

	return core::HashString();
}

//--------------------------------------
// ColladaParser::GetLibraryElementName
//
std::string const& ColladaParser::GetLibraryElementName(core::XML::Element const& libraryEl)
{
	static std::string const s_EmptyName;

	core::XML::Attribute const* attrib = libraryEl.GetAttribute("name"_hash);
	if (attrib != nullptr)
	{
		return attrib->m_Value;
	}

	attrib = libraryEl.GetAttribute("id"_hash);
	if (attrib != nullptr)
	{
		return attrib->m_Value;
	}

	return s_EmptyName;
}

//----------------------------------
// ColladaParser::GetPrimitiveCount
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
			ET_LOG_W(ET_CTX_EDITOR, "Collada data source has asset override, which is currently not supported and may cause problems");
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
			ET_LOG_W(ET_CTX_EDITOR, "Expected data source or accessor, found '%s'.", sourceEl->m_Children[elIdx].m_Name.ToStringDbg());
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
		sources.emplace_back(core::HashString(idAttrib->m_Value.c_str()), dataId, sourceType, accessor, *dataEl);
				
		// next lib
		sourceEl = parent.GetFirstChild(s_SourceHash, ++pos);
	}
}

//------------------------------
// ColladaParser::ResolveSource
//
bool ColladaParser::ResolveSource(dae::Source& source)
{
	ET_ASSERT(source.m_Type != dae::Source::E_Type::None);
	ET_ASSERT(source.m_DataEl != nullptr);

	source.m_TypeSize = dae::Source::GetTypeSize(source.m_Type);
	size_t const typeSize = static_cast<size_t>(source.m_TypeSize);

	core::XML::Attribute const* const countAttrib = source.m_DataEl->GetAttribute("count"_hash);
	if (countAttrib == nullptr)
	{
		ET_LOG_W(ET_CTX_EDITOR, "Expected sources data element to have count element");
		return false;
	}

	size_t const count = static_cast<size_t>(std::stoul(countAttrib->m_Value));
	source.m_Buffer.resize(count * typeSize);

	std::stringstream stream(source.m_DataEl->m_Value);
	size_t idx = 0u;

	switch (source.m_Type)
	{
	case dae::Source::E_Type::IDREF:
	case dae::Source::E_Type::Name:
	{
		std::string val;
		while (stream >> val)
		{
			if (idx > count)
			{
				ET_LOG_W(ET_CTX_EDITOR, "source element count exceeded declared count");
				return false;
			}

			core::HashString const hashVal(val.c_str());
			memcpy(source.m_Buffer.data() + (idx * typeSize), &hashVal, typeSize);

			++idx;
		}
	}
	break;

	case dae::Source::E_Type::Bool:
	{
		bool val;
		while (stream >> val)
		{
			if (idx > count)
			{
				ET_LOG_W(ET_CTX_EDITOR, "source element count exceeded declared count");
				return false;
			}

			memcpy(source.m_Buffer.data() + (idx * typeSize), &val, typeSize);

			++idx;
		}
	}
	break;

	case dae::Source::E_Type::Float:
	{
		float val;
		while (stream >> val)
		{
			if (idx > count)
			{
				ET_LOG_W(ET_CTX_EDITOR, "source element count exceeded declared count");
				return false;
			}

			memcpy(source.m_Buffer.data() + (idx * typeSize), &val, typeSize);

			++idx;
		}
	}
	break;

	case dae::Source::E_Type::Int:
	{
		int64 val;
		while (stream >> val)
		{
			if (idx > count)
			{
				ET_LOG_W(ET_CTX_EDITOR, "source element count exceeded declared count");
				return false;
			}

			memcpy(source.m_Buffer.data() + (idx * typeSize), &val, typeSize);

			++idx;
		}
	}
	break;

	default:
		ET_ASSERT(false, "unhandled source type");
		return false;
	}

	if (idx != count)
	{
		ET_LOG_W(ET_CTX_EDITOR, "source element count didn't match declared count");
	}

	source.m_IsResolved = true;

	return true;
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
		ET_LOG_W(ET_CTX_EDITOR, "Invalid accessor found, no count attribute was present");
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
			ET_LOG_W(ET_CTX_EDITOR, "Expected accessor source attribute to be a URI fragment starting with #, source: '%s'", source->m_Value.c_str());
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
			ET_LOG_W(ET_CTX_EDITOR, "Accessor has invalid parameter without type, skipping parameter");
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
			ET_LOG_W(ET_CTX_EDITOR, "Unrecognized parameter type '%s', skipping parameter", type->m_Value.c_str());
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
			ET_LOG_W(ET_CTX_EDITOR, "Input is missing semantic, skipping");
			inputEl = parent.GetFirstChild(s_InputHash, ++pos);
			continue;
		}

		input.m_Semantic = ReadSemantic(semantic->m_Value);
		if (input.m_Semantic == dae::E_Semantic::Invalid)
		{
			ET_LOG_W(ET_CTX_EDITOR, "Failed to read semantic '%s', skipping input", semantic->m_Value.c_str());
			inputEl = parent.GetFirstChild(s_InputHash, ++pos);
			continue;
		}

		// source points to an accessor
		core::XML::Attribute const* const source = inputEl->GetAttribute("source"_hash);
		if (source == nullptr)
		{
			ET_LOG_W(ET_CTX_EDITOR, "Input is missing source, skipping");
			inputEl = parent.GetFirstChild(s_InputHash, ++pos);
			continue;
		}
		else if ((source->m_Value.empty()) || (source->m_Value[0] != '#'))
		{
			ET_LOG_W(ET_CTX_EDITOR, "Expected input source attribute to be a URI fragment starting with #, source: '%s'", source->m_Value.c_str());
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
				ET_LOG_W(ET_CTX_EDITOR, "Shared input is missing offset, skipping");
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

//--------------------------------------
// ColladaParser::GetMeshElFromGeometry
//
core::XML::Element const* ColladaParser::GetMeshElFromGeometry(core::XML::Element const& geometryEl)
{
	return geometryEl.GetFirstChild("mesh"_hash);
}

//-------------------------
// ColladaParser::ReadMesh
//
bool ColladaParser::ReadMesh(dae::Mesh& mesh, core::XML::Element const& meshEl)
{
	// ensure we have all the relevant XML elements
	//----------------------------------------------
	core::XML::Element const* const verticesEl = meshEl.GetFirstChild("vertices"_hash);
	if (verticesEl == nullptr)
	{
		ET_LOG_W(ET_CTX_EDITOR, "Expected COLLADA mesh to have vertices element!");
		return false;
	}

	core::XML::Attribute const* const vertexIdAttrib = verticesEl->GetAttribute("id"_hash);
	if (vertexIdAttrib == nullptr)
	{
		ET_LOG_W(ET_CTX_EDITOR, "Expected COLLADA vertices to have id attribute!");
		return false;
	}

	core::XML::Element const* vcount = nullptr;
	core::XML::Element const* primitive = meshEl.GetFirstChild("triangles"_hash);
	if (primitive == nullptr)
	{
		primitive = meshEl.GetFirstChild("polylist"_hash);
		if (primitive == nullptr)
		{
			return false; // for now we only support triangle or polylist meshes
		}
		else
		{
			vcount = primitive->GetFirstChild("vcount"_hash);
			if (vcount == nullptr)
			{
				// this is allowed by spec but we don't know default vcounts so can't parse
				ET_LOG_W(ET_CTX_EDITOR, "Expected COLLADA polylist to have vcount element!");
				return false;
			}
		}
	}

	if (GetPrimitiveCount(meshEl) > 1u)
	{
		ET_LOG_W(ET_CTX_EDITOR, "COLLADA mesh had more than one primitive, ignoring subsequent occurances!");
	}

	core::XML::Element const* const primitiveArrayEl = primitive->GetFirstChild("p"_hash);
	if (primitiveArrayEl == nullptr)
	{
		ET_LOG_W(ET_CTX_EDITOR, "Expected COLLADA primitive to have a 'p' element!");
		return false;
	}

	core::XML::Attribute const* const primCountAttrib = primitive->GetAttribute("count"_hash);
	if (primCountAttrib == nullptr)
	{
		ET_LOG_W(ET_CTX_EDITOR, "Expected COLLADA primitive to have a 'count' attribute!");
		return false;
	}

	// setup dataflow
	//----------------
	mesh.m_FaceCount = static_cast<size_t>(std::stoul(primCountAttrib->m_Value));

	ReadSourceList(mesh.m_Sources, meshEl);

	{
		auto resolveInputFn = [&mesh](dae::ResolvedInput& input) -> bool
		{
			// accessor from input
			auto const foundAccessorSourceIt = std::find_if(mesh.m_Sources.cbegin(), mesh.m_Sources.cend(), [&input](dae::Source const& source)
				{
					return (source.m_Id == input.m_Input.m_Source);
				});

			if ((foundAccessorSourceIt == mesh.m_Sources.cend()) || (foundAccessorSourceIt->m_CommonAccessor == nullptr))
			{
				ET_LOG_W(ET_CTX_EDITOR, "Failed to find accessor '%s' for input", input.m_Input.m_Source.ToStringDbg());
				return false;
			}

			input.m_Accessor = foundAccessorSourceIt->m_CommonAccessor;

			// source from accessor
			auto const foundSourceIt = std::find_if(mesh.m_Sources.begin(), mesh.m_Sources.end(), [&input](dae::Source const& source)
				{
					return (source.m_DataId == input.m_Accessor->m_SourceDataId);
				});

			if (foundSourceIt == mesh.m_Sources.cend())
			{
				ET_LOG_W(ET_CTX_EDITOR, "Failed to find source '%s' for accessor", input.m_Accessor->m_SourceDataId.ToStringDbg());
				return false;
			}

			input.m_Source = &(*foundSourceIt);

			if (input.m_Input.m_Offset > mesh.m_MaxInputOffset)
			{
				mesh.m_MaxInputOffset = input.m_Input.m_Offset;
			}

			mesh.m_ResolvedInputs.push_back(input);
			return true;
		};

		std::vector<dae::Input> vertexInputs;
		ReadInputList(vertexInputs, *verticesEl, false);

		std::vector<dae::Input> primitiveInputs;
		ReadInputList(primitiveInputs, *primitive, true);

		core::HashString const verticesId(vertexIdAttrib->m_Value.c_str());
		for (dae::Input const& input : primitiveInputs)
		{
			ET_ASSERT(input.m_Semantic != dae::E_Semantic::Invalid);

			if (input.m_Semantic == dae::E_Semantic::Vertex)
			{
				if (input.m_Source != verticesId)
				{
					ET_LOG_W(ET_CTX_EDITOR, "Expected VERTEX input to use vertices as source");
					return false;
				}

				for (dae::Input const& vertexInput : vertexInputs)
				{
					dae::ResolvedInput resolvedInput;
					resolvedInput.m_Input = vertexInput;
					resolvedInput.m_Input.m_Offset = input.m_Offset;
					resolvedInput.m_Input.m_Set = input.m_Set;

					if (!resolveInputFn(resolvedInput))
					{
						return false;
					}
				}
			}
			else
			{
				dae::ResolvedInput resolvedInput;
				resolvedInput.m_Input = input;

				if (!resolveInputFn(resolvedInput))
				{
					return false;
				}
			}
		}
	}

	ParseArray(mesh.m_PrimitiveIndices, *primitiveArrayEl);

	if (vcount != nullptr)
	{
		ParseArrayU8(mesh.m_VertexCounts, *vcount);
		if (mesh.m_VertexCounts.size() != mesh.m_FaceCount)
		{
			ET_LOG_W(ET_CTX_EDITOR, 
				"Expected vcount array size [" ET_FMT_SIZET "] to be the same as mesh face count [" ET_FMT_SIZET "]",
				mesh.m_VertexCounts.size(),
				mesh.m_FaceCount);
		}
	}

	return true;
}

//-------------------------
// ColladaParser::ReadNode
//
void ColladaParser::ReadNode(dae::Node& node, core::XML::Element const& nodeEl)
{
	// id, name
	core::XML::Attribute const* const nameAttrib = nodeEl.GetAttribute("name"_hash);
	if (nameAttrib != nullptr)
	{
		node.m_Name = nameAttrib->m_Value;
	}

	core::XML::Attribute const* const idAttrib = nodeEl.GetAttribute("id"_hash);
	if (idAttrib != nullptr)
	{
		if (nameAttrib == nullptr)
		{
			node.m_Name = idAttrib->m_Value;
		}

		node.m_Id = core::HashString(idAttrib->m_Value.c_str());
	}

	// asset
	size_t pos = 0u;
	if (nodeEl.GetFirstChild("asset"_hash) != nullptr)
	{
		ET_LOG_W(ET_CTX_EDITOR, "Collada node has asset override, which is currently not supported and may cause problems");
		++pos;
	}

	// transform
	while (ReadTransform(node.m_Transform, nodeEl.m_Children[pos]))
	{
		++pos;
	}

	// instances
	auto const readInstancesFn = [&node, &nodeEl, &pos](dae::Instance::E_Type const type)
	{
		core::HashString const typeId = dae::Instance::GetXmlIdFromType(type);

		core::XML::Element const* instanceEl = nodeEl.GetFirstChild(typeId.Get(), pos);
		while (instanceEl != nullptr)
		{
			core::HashString const url = GetInstanceUrl(*instanceEl);
			if (!url.IsEmpty())
			{
				node.m_Instances.emplace_back(type, url);
			}

			// next instance
			instanceEl = nodeEl.GetFirstChild(typeId.Get(), ++pos);
		}
	};

	readInstancesFn(dae::Instance::E_Type::Geometry);
	readInstancesFn(dae::Instance::E_Type::Node);

	// child nodes
	static T_Hash const s_NodeType = "node"_hash;
	core::XML::Element const* childEl = nodeEl.GetFirstChild(s_NodeType, pos);
	while (childEl != nullptr)
	{
		node.m_Children.emplace_back();
		ReadNode(node.m_Children[node.m_Children.size() - 1u], *childEl);
		
		// next instance
		childEl = nodeEl.GetFirstChild(s_NodeType, ++pos);
	}
}

//--------------------------
// ColladaParser::ReadScene
//
void ColladaParser::ReadScene(dae::VisualScene& scene, core::XML::Element const& sceneEl)
{
	// id
	scene.m_Id = GetElementId(sceneEl);

	// asset
	size_t pos = 0u;
	if (sceneEl.GetFirstChild("asset"_hash) != nullptr)
	{
		ET_LOG_W(ET_CTX_EDITOR, "Collada scene has asset override, which is currently not supported and may cause problems");
		++pos;
	}

	// nodes
	static T_Hash const s_NodeType = "node"_hash;
	core::XML::Element const* childEl = sceneEl.GetFirstChild(s_NodeType, pos);
	while (childEl != nullptr)
	{
		scene.m_Nodes.emplace_back();
		ReadNode(scene.m_Nodes[scene.m_Nodes.size() - 1u], *childEl);

		// next instance
		childEl = sceneEl.GetFirstChild(s_NodeType, ++pos);
	}
}

//------------------------------
// ColladaParser::ReadTransform
//
// Read transform elements, return true if the element was a transform element, and multiply into matrix
//
bool ColladaParser::ReadTransform(mat4& mat, core::XML::Element const& transformEl)
{
	switch (transformEl.m_Name.Get())
	{
	case "lookat"_hash:
	{
		std::vector<float> values;
		ParseArray(values, transformEl);
		if (values.size() != 9u)
		{
			ET_LOG_W(ET_CTX_EDITOR, "lookat transform expected 9 values, found " ET_FMT_SIZET, values.size());
			return true;
		}

		mat = mat * math::lookAt(vec3(values[0], values[1], values[2]), vec3(values[3], values[4], values[5]), vec3(values[6], values[7], values[8]));
	}
	return true;

	case "matrix"_hash:
	{
		std::vector<float> values;
		ParseArray(values, transformEl);
		if (values.size() != 16u)
		{
			ET_LOG_W(ET_CTX_EDITOR, "matrix transform expected 16 values, found " ET_FMT_SIZET, values.size());
			return true;
		}

		mat = mat * mat4({values[0], values[1], values[2], values[3],
			values[4], values[5], values[6], values[7],
			values[8], values[9], values[10], values[11],
			values[12], values[13], values[14], values[15]});
	}
	return true;

	case "rotate"_hash:
	{
		std::vector<float> values;
		ParseArray(values, transformEl);
		if (values.size() != 4u)
		{
			ET_LOG_W(ET_CTX_EDITOR, "rotation transform expected 4 values, found " ET_FMT_SIZET, values.size());
			return true;
		}

		math::rotate(mat, vec3(values[0], values[1], values[2]), math::radians(values[3]));
	}
	return true;

	case "scale"_hash:
	{
		std::vector<float> values;
		ParseArray(values, transformEl);
		if (values.size() != 3u)
		{
			ET_LOG_W(ET_CTX_EDITOR, "scale transform expected 3 values, found " ET_FMT_SIZET, values.size());
			return true;
		}

		math::scale(mat, vec3(values[0], values[1], values[2]));
	}
	return true;

	case "skew"_hash:
	{
		ET_LOG_W(ET_CTX_EDITOR, "COLLADA Skew transform was found but is currently not supported");
	}
	return true;

	case "translate"_hash:
	{
		std::vector<float> values;
		ParseArray(values, transformEl);
		if (values.size() != 3u)
		{
			ET_LOG_W(ET_CTX_EDITOR, "translation transform expected 3 values, found " ET_FMT_SIZET, values.size());
			return true;
		}

		math::translate(mat, vec3(values[0], values[1], values[2]));
	}
	return true;
	}

	return false;
}

//-----------------------------
// ColladaParser::ParseArrayU8
//
// Needs specialization because ParseArray would interpret uint8 as a char
//
void ColladaParser::ParseArrayU8(std::vector<uint8>& vec, core::XML::Element const& el)
{
	std::stringstream stream(el.m_Value);

	uint16 val;
	while (stream >> val)
	{
		vec.push_back(static_cast<uint8>(val));
	}
}


} // namespace edit
} // namespace et
