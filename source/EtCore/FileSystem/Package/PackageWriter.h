#pragma once

class File;
class Directory;


//---------------------------------
// PackageWriter
//
// Writes a list of files to a binary package/archive 
//
class PackageWriter final
{
public:
	~PackageWriter();

	void AddDirectory(Directory* const directory);

	void Write(std::vector<uint8>& data);

private:
	std::vector<File*> m_Files;
};

