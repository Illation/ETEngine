#pragma once
#include <EtPipeline/Content/EditorAssetDatabase.h>

#include "PackageWriter.h"



namespace et { 
namespace cooker {


int RunCooker(int argc, char *argv[]);

void AddPackageToWriter(core::HashString const packageId,
	std::string const& dbBase,
	PackageWriter &writer,
	pl::EditorAssetDatabase& db,
	bool const setupRuntime);

void CookCompiledPackage(std::string const& dbBase,
	std::string const& outPath,
	std::string const& resName,
	pl::EditorAssetDatabase& db,
	std::string const& engineDbBase,
	pl::EditorAssetDatabase& engineDb);

void CookFilePackages(std::string const& dbBase,
	std::string const& outPath,
	pl::EditorAssetDatabase& db,
	std::string const& engineDbBase,
	pl::EditorAssetDatabase& engineDb);


} // namespace cooker
} // namespace et
