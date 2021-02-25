#pragma once
#include <iosfwd>


namespace et {
namespace edit {


//================
// Collada Parser
//================


//-----------------------------
// ColladaParser::ParseArray
//
template <typename TDataType>
void ColladaParser::ParseArray(std::vector<TDataType>& vec, core::XML::Element const& el)
{
	std::stringstream stream(el.m_Value);

	TDataType val;
	while (stream >> val)
	{
		vec.push_back(val);
	}
}


} // namespace edit
} // namespace et

