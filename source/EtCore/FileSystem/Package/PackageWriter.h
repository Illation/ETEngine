#pragma once
#include "PackageDataStructure.h"

class File;


//---------------------------------
// PackageWriter
//
// Writes a list of files to a binary package/archive 
//
class PackageWriter final
{
public:
	typedef std::pair<PkgEntry, File*> T_EntryFilePair;

	~PackageWriter();

	void AddFile(File* const file, E_CompressionType const compression);

	void Write(std::vector<uint8>& data);

private:
	std::vector<T_EntryFilePair> m_Files;
};

