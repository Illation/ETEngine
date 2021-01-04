#pragma once


namespace et { namespace core {
	struct TypeInfo;
	class BinaryWriter;
} }


namespace et {
namespace core {


//------------------
// BinarySerializer
//
// Class that contains all functionality to serialize a reflected object into an array of bytes
//
class BinarySerializer final
{
	// construct
	//-----------
public:
	BinarySerializer(bool const isVerbose = false);

	// functionality
	//---------------
	template<typename TDataType>
	bool SerializeToData(TDataType const& serialObject, std::vector<uint8>& outData);

	// utility 
	//---------
private:
	bool SerializeRoot(rttr::variant const& var, std::vector<uint8>& outData);

	// general
	bool WriteVariant(rttr::variant const& var);

	// atomic
	bool WriteArithmeticType(rttr::type const type, rttr::variant const& var);
	bool WriteVectorType(rttr::type const type, rttr::variant const& var);
	void WriteString(std::string const& str);
	void WriteHash(HashString const hash);

	// complex
	bool WriteSequentialContainer(rttr::variant_sequential_view const& view);
	bool WriteAssociativeContainer(rttr::variant_associative_view const& view);
	bool WriteObject(rttr::instance const& inst, TypeInfo const& ti);

	// Data
	///////

	bool const m_IsVerbose;
	BinaryWriter* m_Writer = nullptr;
};


} // namespace core
} // namespace et


#include "BinarySerializer.inl"
