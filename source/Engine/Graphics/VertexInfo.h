#pragma once


typedef uint8 T_VertexFlags;

//---------------------------------
// E_VertexFlag
//
// Bitflags specifying what vertex info should be present for mesh shaders
//
enum E_VertexFlag : T_VertexFlags
{
	POSITION = 1 << 0,
	NORMAL   = 1 << 1,
	BINORMAL = 1 << 2,
	TANGENT  = 1 << 3,
	COLOR    = 1 << 4,
	TEXCOORD = 1 << 5
};

//---------------------------------
// AttributeDescriptor
//
// Per vertex type data for automatic input layout definition
//
struct AttributeDescriptor
{
	// Definitions
	//-------------

	//---------------------------------
	// E_DataType
	//
	// Per attribute data type
	//
	enum class E_DataType
	{
		Byte,
		UByte,
		Short,
		UShort,
		Int,
		UInt,

		Half,
		Float,
		Double
	};

	static std::map<E_VertexFlag, AttributeDescriptor const> const s_VertexAttributes;

	// static functionality
	//----------------------
	static std::string PrintFlags(T_VertexFlags const flags);
	static uint16 GetVertexSize(T_VertexFlags const flags);
	static bool ValidateFlags(T_VertexFlags const supportedFlags, T_VertexFlags const requiredFlags);
	static void DefineAttributeArray(T_VertexFlags const flags, std::vector<int32> const& locations);

	// accessors
	//-----------
	uint32 GetDataType() const;
	uint16 GetDataSize() const;

	// Data
	///////

	std::string name;
	E_DataType dataType;
	uint32 dataCount;
};

