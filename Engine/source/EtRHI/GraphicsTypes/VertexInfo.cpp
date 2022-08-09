#include "stdafx.h"
#include "VertexInfo.h"


namespace et {
namespace rhi {


//================
// Data Type Info
//================


// static definition for vertex data types
std::vector<DataTypeInfo> const DataTypeInfo::s_DataTypes =
{
	{ E_DataType::Byte,		1 },
	{ E_DataType::UByte,	1 },
	{ E_DataType::Short,	2 },
	{ E_DataType::UShort,	2 },
	{ E_DataType::Int,		4 },
	{ E_DataType::UInt,		4 },
	{ E_DataType::Half,		2 },
	{ E_DataType::Float,	4 },
	{ E_DataType::Double,	8 }
};


//-------------------------------------------
// AttributeDescriptor::GetTypeSize
//
// Gets the size in bytes of an attribute data type
//
uint16 DataTypeInfo::GetTypeSize(E_DataType const dataType)
{
	auto const typeIt = std::find_if(s_DataTypes.cbegin(), s_DataTypes.cend(), [dataType](DataTypeInfo const& typeInfo)
	{
		return typeInfo.type == dataType;
	});

	if (typeIt == s_DataTypes.cend())
	{
		ET_ERROR("Data type not implemented!");
		return 0u;
	}

	return typeIt->size;
}


//======================
// Attribute Descriptor
//======================


// static definition for how vertex data should be laid out in a shader and mesh
std::map<E_VertexFlag, AttributeDescriptor const> const AttributeDescriptor::s_VertexAttributes =
{
	{ E_VertexFlag::POSITION,	{ "position",	E_DataType::Float, 3 } },
	{ E_VertexFlag::NORMAL,		{ "normal",		E_DataType::Float, 3 } },
	{ E_VertexFlag::BINORMAL,	{ "binormal",	E_DataType::Float, 3 } },
	{ E_VertexFlag::TANGENT,	{ "tangent",	E_DataType::Float, 3 } },
	{ E_VertexFlag::COLOR,		{ "color",		E_DataType::Float, 4 } },
	{ E_VertexFlag::TEXCOORD,	{ "texcoord",	E_DataType::Float, 2 } }
};


//---------------------------------
// AttributeDescriptor::PrintFlags
//
// Converts Vertex Flags into a string
//
std::string AttributeDescriptor::PrintFlags(T_VertexFlags const flags)
{
	std::string flagstring;

	for (auto const attributeIt : AttributeDescriptor::s_VertexAttributes)
	{
		if (flags & attributeIt.first)
		{
			flagstring += attributeIt.second.name + std::string(", ");
		}
	}

	return flagstring;
}

//-------------------------------------
// AttributeDescriptor::GetVertexSize
//
// Size in bytes for a vertex using flags 
//
uint16 AttributeDescriptor::GetVertexSize(T_VertexFlags const flags)
{
	uint16 size = 0u;

	for (auto const attributeIt : AttributeDescriptor::s_VertexAttributes)
	{
		if (flags & attributeIt.first)
		{
			size += attributeIt.second.dataCount * DataTypeInfo::GetTypeSize(attributeIt.second.dataType);
		}
	}

	return size;
}

//-------------------------------------
// AttributeDescriptor::GetVertexSize
//
// Checks if required flags are supported
//
bool AttributeDescriptor::ValidateFlags(T_VertexFlags const supportedFlags, T_VertexFlags const requiredFlags)
{
	for (auto const attributeIt : AttributeDescriptor::s_VertexAttributes)
	{
		if ((requiredFlags & attributeIt.first) && !(supportedFlags & attributeIt.first))
		{
			return false;
		}
	}

	return true;
}

//-------------------------------------------
// AttributeDescriptor::DefineAttributeArray
//
// Enables vertex attributes for flags, given a list of locations matching the number of on flags in order of E_VertexFlag
//
void AttributeDescriptor::DefineAttributeArray(T_VertexFlags const flags, std::vector<int32> const& locations)
{
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	uint16 const stride = AttributeDescriptor::GetVertexSize(flags);

	uint32 startPos = 0u;
	size_t locationIdx = 0u;
	for (auto it = AttributeDescriptor::s_VertexAttributes.begin(); it != AttributeDescriptor::s_VertexAttributes.end(); ++it)
	{
		if (flags & it->first) // check this attribute is being used
		{
			ET_ASSERT(locationIdx < locations.size());

			device->SetVertexAttributeArrayEnabled(locations[locationIdx], true);
			device->DefineVertexAttributePointer(locations[locationIdx],
				it->second.dataCount,
				it->second.dataType,
				false,
				stride,
				static_cast<size_t>(startPos));

			startPos += it->second.dataCount * DataTypeInfo::GetTypeSize(it->second.dataType); // the next attribute starts after this one
			++locationIdx;
		}
	}

	// make sure all flags had locations and vice versa
	ET_ASSERT(locationIdx == locations.size());
}

//-------------------------------------------
// AttributeDescriptor::DefineAttributeArray
//
// Enables vertex attributes for flags, given a list of locations matching the number of on flags in order of E_VertexFlag
//  - the locations are defined as a subset of supported vertex types, assuming all supported types are in a buffer on the GPU
//
void AttributeDescriptor::DefineAttributeArray(T_VertexFlags const supportedFlags, T_VertexFlags const targetFlags, std::vector<int32> const& locations)
{
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	uint16 const stride = AttributeDescriptor::GetVertexSize(supportedFlags);

	uint32 startPos = 0u;
	size_t locationIdx = 0u;
	for (auto it = AttributeDescriptor::s_VertexAttributes.begin(); it != AttributeDescriptor::s_VertexAttributes.end(); ++it)
	{
		if (supportedFlags & it->first) // if the flag is supported we need to at least increment the pointer offset
		{
			if (targetFlags & it->first) 
			{
				ET_ASSERT(locationIdx < locations.size());

				device->SetVertexAttributeArrayEnabled(locations[locationIdx], true);
				device->DefineVertexAttributePointer(locations[locationIdx],
					it->second.dataCount,
					it->second.dataType,
					false,
					stride,
					static_cast<size_t>(startPos));

				++locationIdx;
			}

			startPos += it->second.dataCount * DataTypeInfo::GetTypeSize(it->second.dataType); // the next attribute starts after this one
		}
		else
		{
			ET_ASSERT(!(targetFlags & it->first), "Supported flags don't include '%s' - input layout will be invalid", it->second.name.c_str());
		}
	}

	// make sure all flags had locations and vice versa
	ET_ASSERT(locationIdx == locations.size());
}

//-------------------------------------------
// AttributeDescriptor::GetVertexFlag
//
// Tries retrieving a vertex flag based on its name
//
bool AttributeDescriptor::GetVertexFlag(AttributeDescriptor const& desc, E_VertexFlag& flag)
{
	auto const attIt = std::find_if(s_VertexAttributes.cbegin(), s_VertexAttributes.cend(), 
		[desc](std::pair<E_VertexFlag, AttributeDescriptor const> const& vertAttrib)
		{
			return vertAttrib.second.name == desc.name;
		});

	if (attIt == s_VertexAttributes.cend())
	{
		return false;
	}

	flag = attIt->first;
	return true;
}


} // namespace rhi
} // namespace et
