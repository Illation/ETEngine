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
			size += attributeIt.second.dataCount * attributeIt.second.GetDataSize();
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
				it->second.GetDataType(),
				GL_FALSE,
				stride,
				static_cast<char const*>(0) + startPos);

			startPos += it->second.dataCount * it->second.GetDataSize(); // the next attribute starts after this one
			++locationIdx;
		}
	}

	// make sure all flags had locations and vice versa
	ET_ASSERT(locationIdx == locations.size());
}

//-------------------------------------------
// AttributeDescriptor::GetDataType
//
// Converts an attribute data type into the OpenGL equivalent
//
uint32 AttributeDescriptor::GetDataType() const
{
	switch (dataType)
	{
	case E_DataType::Byte:
		return GL_BYTE;

	case E_DataType::UByte:
		return GL_UNSIGNED_BYTE;

	case E_DataType::Short:
		return GL_SHORT;

	case E_DataType::UShort:
		return GL_UNSIGNED_SHORT;

	case E_DataType::Int:
		return GL_INT;

	case E_DataType::UInt:
		return GL_UNSIGNED_INT;

	case E_DataType::Half:
		return GL_HALF_FLOAT;

	case E_DataType::Float:
		return GL_FLOAT;

	case E_DataType::Double:
		return GL_DOUBLE;
	}

	ET_ASSERT(false, "Data type not implemented!");
	return GL_NONE;
}

//-------------------------------------------
// AttributeDescriptor::GetDataSize
//
// Gets the size in bytes of an attribute data type
//
uint16 AttributeDescriptor::GetDataSize() const
{
	switch (dataType)
	{
	case E_DataType::Byte:
		return 1u;

	case E_DataType::UByte:
		return 1u;

	case E_DataType::Short:
		return 2u;

	case E_DataType::UShort:
		return 2u;

	case E_DataType::Int:
		return 4u;

	case E_DataType::UInt:
		return 4u;

	case E_DataType::Half:
		return 2u;

	case E_DataType::Float:
		return 4u;

	case E_DataType::Double:
		return 8u;
	}

	ET_ASSERT(false, "Data type not implemented!");
	return 0u;
}
