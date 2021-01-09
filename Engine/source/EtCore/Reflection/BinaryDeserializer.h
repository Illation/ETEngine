#pragma once
#include <EtCore/IO/BinaryReader.h>

#include "TypeInfoRegistry.h"


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
	bool DeserializeRoot(rttr::instance& inst, TypeInfo const& ti, std::vector<uint8> const& data);
	bool InitFromHeader(std::vector<uint8> const& data, rttr::type const callingType);

	// general
	bool ReadVariant(rttr::variant& var, rttr::type const callingType);
	bool ReadBasicVariant(rttr::variant& var, TypeInfo const& ti);

	// atomic
	bool ReadArithmeticType(rttr::variant& var, HashString const typeId);
	bool ReadVectorType(rttr::variant& var, HashString const typeId);
	void ReadHash(rttr::variant& var);

	// complex
	bool ReadSequentialContainer(rttr::variant& var);
	bool ReadAssociativeContainer(rttr::variant& var);
	bool ReadObject(rttr::variant& var, TypeInfo const& ti);
	bool ReadObjectProperties(rttr::instance& inst, TypeInfo const& ti, uint16 const propCount);


	// Data
	///////

	bool m_IsVerbose;
	BinaryReader m_Reader;
};


} // namespace core
} // namespace et


#include "BinaryDeserializer.inl"
