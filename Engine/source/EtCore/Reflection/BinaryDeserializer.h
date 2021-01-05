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

	// atomic
	bool ReadArithmeticType(rttr::variant& var, rttr::type const type);
	bool ReadVectorType(rttr::variant& var, HashString const typeId);
	void ReadHash(rttr::variant& var);

	// complex
	bool ReadSequentialContainer(rttr::variant& var);
	bool ReadAssociativeContainer(rttr::variant& var);
	bool ReadObject(rttr::variant& var, TypeInfo const& ti);

	// misc
	template <uint8 n, class T>
	math::vector<n, T> ReadVector();

	template <uint8 m, uint8 n, class T>
	math::matrix<m, n, T> ReadMatrix();

	// Data
	///////

	bool m_IsVerbose;
	BinaryReader m_Reader;
};


} // namespace core
} // namespace et


#include "BinaryDeserializer.inl"
