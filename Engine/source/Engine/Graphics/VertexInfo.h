#pragma once


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
	Double,

	Invalid
};

//---------------------------------
// DataTypeInfo
//
// Per DataType information
//
struct DataTypeInfo
{
	// Definitions
	//-------------
	static std::vector<DataTypeInfo> const s_DataTypes;

	// static functionality
	//----------------------
	static uint32 GetTypeId(E_DataType const dataType);
	static uint16 GetTypeSize(E_DataType const dataType);
	static E_DataType GetDataType(uint32 const typeId);

	// Data
	///////

	E_DataType type;
	uint32 typeId;
	uint16 size;
};

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
	static std::map<E_VertexFlag, AttributeDescriptor const> const s_VertexAttributes;

	// static functionality
	//----------------------
	static std::string PrintFlags(T_VertexFlags const flags);
	static uint16 GetVertexSize(T_VertexFlags const flags);
	static bool ValidateFlags(T_VertexFlags const supportedFlags, T_VertexFlags const requiredFlags);
	static void DefineAttributeArray(T_VertexFlags const flags, std::vector<int32> const& locations);
	static void DefineAttributeArray(T_VertexFlags const supportedFlags, T_VertexFlags const targetFlags, std::vector<int32> const& locations);
	static bool GetVertexFlag(AttributeDescriptor const& desc, E_VertexFlag& flag);

	// Data
	///////

	std::string name;
	E_DataType dataType;
	uint32 dataCount;
};

