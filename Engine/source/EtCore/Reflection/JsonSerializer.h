#pragma once


namespace et { namespace core { namespace JSON {
	struct Value;
}
	struct TypeInfo;
} 
	class I_AssetPtr;
}


namespace et {
namespace core {


//----------------
// JsonSerializer
//
// Class that contains all functionality to serialize a reflected object into the JSON DOM
//
class JsonSerializer final 
{
	// construct
	//-----------
public:
	JsonSerializer(bool const isVerbose = true) : m_IsVerbose(isVerbose) { }

	// functionality
	//---------------
	template<typename TDataType>
	bool SerializeToData(TDataType const& serialObject, std::vector<uint8>& data);

	template<typename TDataType>
	JSON::Value* Serialize(TDataType const& serialObject);

	// utility 
	//---------
private:
	bool SerializeRoot(rttr::variant const& var, JSON::Value*& outVal);

	// general
	bool WriteVariant(rttr::variant const& var, JSON::Value*& outVal);
	bool WriteBasicVariant(rttr::variant const& var, TypeInfo const& ti, JSON::Value*& outVal);

	// atomic
	bool WriteArithmeticType(rttr::type const type, rttr::variant const& var, JSON::Value*& outVal);
	bool WriteVectorType(rttr::type const type, rttr::variant const& var, JSON::Value*& outVal);
	bool WriteEnum(rttr::variant const& var, JSON::Value*& outVal);
	void WriteAssetPtr(I_AssetPtr const& ptr, JSON::Value*& outVal);
	void WriteString(std::string const& str, JSON::Value*& outVal);
	void WriteHash(HashString const hash, JSON::Value*& outVal);

	// complex
	bool WriteSequentialContainer(rttr::variant_sequential_view const& view, JSON::Value*& outVal);
	bool WriteAssociativeContainer(rttr::variant_associative_view const& view, JSON::Value*& outVal);
	bool WriteObject(rttr::instance const& inst, TypeInfo const& ti, JSON::Value*& outVal);

	// Data
	///////

	bool m_IsVerbose = true;
};


} // namespace core
} // namespace et


#include "JsonSerializer.inl"
