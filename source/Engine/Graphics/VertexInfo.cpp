#include "stdafx.h"
#include "VertexInfo.h"


//======================
// Attribute Descriptor
//======================


// statics

// definition for how vertex data should be laid out in a shader and mesh
std::map<E_VertexFlag, AttributeDescriptor const> const AttributeDescriptor::s_VertexAttributes =
{
	{ E_VertexFlag::POSITION,	{ "position",	AttributeDescriptor::E_DataType::Float, 3 } },
	{ E_VertexFlag::NORMAL,		{ "normal",		AttributeDescriptor::E_DataType::Float, 3 } },
	{ E_VertexFlag::BINORMAL,	{ "binormal",	AttributeDescriptor::E_DataType::Float, 3 } },
	{ E_VertexFlag::TANGENT,	{ "tangent",	AttributeDescriptor::E_DataType::Float, 3 } },
	{ E_VertexFlag::COLOR,		{ "color",		AttributeDescriptor::E_DataType::Float, 3 } },
	{ E_VertexFlag::TEXCOORD,	{ "texcoord",	AttributeDescriptor::E_DataType::Float, 2 } }
};

// definition for vertex data types
std::vector<AttributeDescriptor::DataTypeInfo> const AttributeDescriptor::s_DataTypes =
{
	{ AttributeDescriptor::E_DataType::Byte,	GL_BYTE,			1 },
	{ AttributeDescriptor::E_DataType::UByte,	GL_UNSIGNED_BYTE,	1 },
	{ AttributeDescriptor::E_DataType::Short,	GL_SHORT,			2 },
	{ AttributeDescriptor::E_DataType::UShort,	GL_UNSIGNED_SHORT,	2 },
	{ AttributeDescriptor::E_DataType::Int,		GL_INT,				4 },
	{ AttributeDescriptor::E_DataType::UInt,	GL_UNSIGNED_INT,	4 },
	{ AttributeDescriptor::E_DataType::Half,	GL_HALF_FLOAT,		2 },
	{ AttributeDescriptor::E_DataType::Float,	GL_FLOAT,			4 },
	{ AttributeDescriptor::E_DataType::Double,	GL_DOUBLE,			8 }
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
			size += attributeIt.second.dataCount * GetTypeSize(attributeIt.second.dataType);
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
	uint16 const stride = AttributeDescriptor::GetVertexSize(flags);

	uint32 startPos = 0u;
	size_t locationIdx = 0u;
	for (auto it = AttributeDescriptor::s_VertexAttributes.begin(); it != AttributeDescriptor::s_VertexAttributes.end(); ++it)
	{
		if (flags & it->first) // check this attribute is being used
		{
			ET_ASSERT(locationIdx < locations.size());

			glEnableVertexAttribArray(locations[locationIdx]);
			glVertexAttribPointer(locations[locationIdx],
				it->second.dataCount,
				GetTypeId(it->second.dataType),
				GL_FALSE,
				stride,
				static_cast<char const*>(0) + startPos);

			startPos += it->second.dataCount * GetTypeSize(it->second.dataType); // the next attribute starts after this one
			++locationIdx;
		}
	}

	// make sure all flags had locations and vice versa
	ET_ASSERT(locationIdx == locations.size());
}

//-------------------------------------------
// AttributeDescriptor::GetTypeId
//
// Converts an attribute data type into the OpenGL equivalent
//
uint32 AttributeDescriptor::GetTypeId(E_DataType const dataType)
{
	auto const typeIt = std::find_if(s_DataTypes.cbegin(), s_DataTypes.cend(), [dataType](DataTypeInfo const& typeInfo)
		{
			return typeInfo.type == dataType;
		});

	if (typeIt == s_DataTypes.cend())
	{
		ET_ASSERT(false, "Data type not implemented!");
		return 0u;
	}

	return typeIt->typeId;
}

//-------------------------------------------
// AttributeDescriptor::GetTypeSize
//
// Gets the size in bytes of an attribute data type
//
uint16 AttributeDescriptor::GetTypeSize(E_DataType const dataType)
{
	auto const typeIt = std::find_if(s_DataTypes.cbegin(), s_DataTypes.cend(), [dataType](DataTypeInfo const& typeInfo)
		{
			return typeInfo.type == dataType;
		});

	if (typeIt == s_DataTypes.cend())
	{
		ET_ASSERT(false, "Data type not implemented!");
		return 0u;
	}

	return typeIt->size;
}

//-------------------------------------------
// AttributeDescriptor::GetDataType
//
// Gets a data type from its ID
//
AttributeDescriptor::E_DataType AttributeDescriptor::GetDataType(uint32 const typeId)
{
	auto const typeIt = std::find_if(s_DataTypes.cbegin(), s_DataTypes.cend(), [typeId](DataTypeInfo const& typeInfo)
		{
			return typeInfo.typeId == typeId;
		});

	if (typeIt == s_DataTypes.cend())
	{
		ET_ASSERT(false, "Data type not implemented!");
		return E_DataType::Invalid;
	}

	return typeIt->type;
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
