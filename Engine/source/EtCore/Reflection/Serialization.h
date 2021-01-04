#pragma once


namespace et {
namespace core {


//---------------------------------
// serialization
//
// This namespace will contain all functionality to serialize and deserialize things to various data formats, for now json
//
namespace serialization
{
	// Serialization
	//----------------

	template<typename T>
	bool SerializeToFile(std::string const& filePath, T const& serialObject, bool const verbose);

	// Deserialization
	//-----------------

	template<typename T>
	bool DeserializeFromFile(std::string const& filePath, T& outObject);

} // namespace serialization


} // namespace core
} // namespace et

#include "Serialization.inl"
