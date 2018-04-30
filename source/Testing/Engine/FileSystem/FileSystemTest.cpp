#include "../../../Engine/stdafx.hpp"
#include <catch.hpp>

#include "../../../Engine/FileSystem/Entry.h"
#include "../../../Engine/FileSystem/FileUtil.h"
#include <thread>
#include <chrono>

TEST_CASE( "mount", "[filesystem]" )
{
	std::string dirName = "./source/Testing/Engine/FileSystem/TestDir/";
	Directory* pDir = new Directory( dirName, nullptr );

	REQUIRE( pDir->IsMounted() == false );

	bool mountResult = pDir->Mount( true );
	REQUIRE( mountResult == true );
	REQUIRE( pDir->IsMounted() == true );

	std::vector<Entry*> extChildren = pDir->GetChildrenByExt( "someFileExt" );
	bool found = false;
	for(auto cExt : extChildren)
	{
		if(cExt->GetType() == Entry::EntryType::ENTRY_FILE)
		{
			REQUIRE( cExt->GetName() == "./test_file3.someFileExt" );
			found = true;
		}
	}
	REQUIRE( found == true );

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

//this is a long test but a lot of the feature testing needs to work in a sequence
//if this test fails the test directory might need to be rebuilt or repaired manually
TEST_CASE( "copy file", "[filesystem]" )
{
	std::string expectedContent = "Hello I am a test file!\r\nwith 2 lines\r\n";
	auto expectedContentLines = FileUtil::ParseLines(expectedContent);

	std::string dirName = "./source/Testing/Engine/FileSystem/TestDir/";
	Directory* pDir = new Directory( dirName, nullptr );
	bool mountResult = pDir->Mount( true );
	REQUIRE( mountResult == true );

	File* inputFile = nullptr;
	Directory* subDir = nullptr;
	for(auto c : pDir->GetChildren())
	{
		if(c->GetName() == "./test_file.txt")
		{
			inputFile = static_cast<File*>(c);
		}
		if(c->GetType() == Entry::EntryType::ENTRY_DIRECTORY)
		{
			subDir = static_cast<Directory*>(c);
		}
	}
	REQUIRE_FALSE( inputFile == nullptr );
	REQUIRE_FALSE( subDir == nullptr );

	/* Create input file descriptor */
	File* outputFile = new File( "copy_file.txt", subDir );

	bool openResult = inputFile->Open( FILE_ACCESS_MODE::Read );
	REQUIRE( openResult == true );

	/* Create output file descriptor */
	FILE_ACCESS_FLAGS outFlags;
	outFlags.SetFlags( FILE_ACCESS_FLAGS::FLAGS::Create | FILE_ACCESS_FLAGS::FLAGS::Exists );
	bool openResult2 = outputFile->Open( FILE_ACCESS_MODE::Write, outFlags );
	REQUIRE( openResult2 == true );

	/* Copy process */
	std::string content = FileUtil::AsText(inputFile->Read());
	REQUIRE( FileUtil::ParseLines(content) == expectedContentLines );

	bool writeResult = outputFile->Write( FileUtil::FromText(content) );
	REQUIRE( writeResult == true );

	//for some reason need to close and reopen
	outputFile->Close();
	bool openResult3 = outputFile->Open( FILE_ACCESS_MODE::Read );
	REQUIRE( openResult3 == true );
	////Wait for file to be written (ewwww)
	//std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );

	/* Copy process */
	std::string contentOut = FileUtil::AsText(outputFile->Read());
	REQUIRE( FileUtil::ParseLines(contentOut) == expectedContentLines );
	//outputFile->Close();

	/* Close file descriptors */
	pDir->Unmount();
	subDir = nullptr;
	inputFile = nullptr;
	outputFile = nullptr;

	// reopen and see if the file is there
	bool mountResult2 = pDir->Mount( true );
	REQUIRE( mountResult2 == true );
	for(auto c : pDir->GetChildren())
	{
		if(c->GetType() == Entry::EntryType::ENTRY_DIRECTORY)
		{
			subDir = static_cast<Directory*>(c);
			break;
		}
	}
	REQUIRE_FALSE( subDir == nullptr );
	for(auto c : subDir->GetChildren())
	{
		if(c->GetName() == "./copy_file.txt")
		{
			outputFile = static_cast<File*>(c);
			break;
		}
	}
	REQUIRE_FALSE( outputFile == nullptr );
	bool openResult4 = outputFile->Open( FILE_ACCESS_MODE::Read );
	REQUIRE( openResult4 == true );
	std::string contentOut2 = FileUtil::AsText(outputFile->Read());
	REQUIRE( FileUtil::ParseLines(contentOut2) == expectedContentLines );

	outputFile->Close();

	//Deleting should handle closing the file
	bool deleteResult = outputFile->Delete();
	REQUIRE( deleteResult == true );
	deleteResult = nullptr;

	//The file should be gone from the directory
	bool found = false;
	for(auto c : subDir->GetChildren())
	{
		if(c->GetNameOnly() == "copy_file.txt")
			found = true;
	}
	REQUIRE( found == false );

	//Should also hold true after remounting
	subDir->Unmount();
	bool mountResult3 = subDir->Mount( true );
	REQUIRE( mountResult3 == true );
	bool found2 = false;
	for(auto c : subDir->GetChildren())
	{
		if(c->GetNameOnly() == "copy_file.txt")
			found2 = true;
	}
	REQUIRE( found == false );

	//Deleting the parent directory should close all other stuff
	delete pDir;
	subDir = nullptr;
	outputFile = nullptr;
	pDir = nullptr;
}
