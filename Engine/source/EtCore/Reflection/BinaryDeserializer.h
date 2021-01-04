#pragma once
#include <EtCore/IO/BinaryReader.h>


namespace et { namespace core {
	struct TypeInfo;
} }


namespace et {
namespace core {


//--------------------
// BinaryDeserializer
//
// Class that contains all functionality to deserialize a reflected object from an etbin file
//
class BinaryDeserializer final
{
	// construct
	//-----------
public:
	BinaryDeserializer() = default;

	// functionality
	//---------------
	template<typename TDataType>
	bool DeserializeFromData(std::vector<uint8> const& data, TDataType& outObject);

	// utility 
	//---------
private:
	bool DeserializeRoot(rttr::variant& var, rttr::type const callingType, std::vector<uint8> const& data);

	// general
	bool ReadVariant(rttr::variant& var, rttr::type const callingType);
	bool ReadBasicVariant(rttr::variant& var, TypeInfo const& ti);

	// Data
	///////

	bool m_IsVerbose;
	BinaryReader m_Reader;
};


} // namespace core
} // namespace et


#include "BinaryDeserializer.inl"
