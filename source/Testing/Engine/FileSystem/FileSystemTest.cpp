#include <catch.hpp>

#include "../../../Engine/FileSystem/Entry.h"

TEST_CASE( "mount", "[filesystem]" )
{
	std::string dirName = "./Testing/Engine/FileSystem/TestDir/";
	Directory* pDir = new Directory( dirName, nullptr );

	REQUIRE( pDir->IsMounted() == false );

	bool mountResult = pDir->Mount( true );
	REQUIRE( mountResult == true );
	REQUIRE( pDir->IsMounted() == true );

	std::vector<Entry*> extChildren = pDir->GetChildrenByExt( "someFileExt" );
	for(auto cExt : extChildren)
	{
		if(cExt->GetType() == Entry::EntryType::ENTRY_FILE)
		{
			REQUIRE( cExt->GetName() == "testFile3.someFileExt" );
		}
	}

	for(auto c : pDir->GetChildren())
	{
		if(c->GetType() == Entry::EntryType::ENTRY_DIRECTORY)
		{
			auto cDir = static_cast<Directory*>(c);
			REQUIRE( cDir->IsMounted() == true );
			REQUIRE( cDir->GetChildren().size() == 1 );
		}
	}

	pDir->Unmount();
	REQUIRE( pDir->IsMounted() == false );
	REQUIRE( pDir->GetChildren().size() == 0 );

	delete pDir;
	pDir = nullptr;
}