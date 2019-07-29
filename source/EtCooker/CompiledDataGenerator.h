#pragma once

#include <vector>
#include <string>

#include <EtCore/Helper/AtomicTypes.h>


void GenerateCompilableResource(std::vector<uint8> const& data, std::string const& name, std::string const& path);

//---------------------------------
// generator_detail
//
// Functionality for compiling data into a string
//
namespace generator_detail {

	std::string GetHeaderString(std::string const& name, std::string const& compiledDataName);
	std::string GetSourceString(std::vector<uint8> const& data, std::string const& name, std::string const& compiledDataName);

} // namespace generator_detail
