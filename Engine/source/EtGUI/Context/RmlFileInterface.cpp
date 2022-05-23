#include "stdafx.h"
#include "RmlFileInterface.h"

#include <stdio.h>

#include <EtCore/Content/ResourceManager.h>


namespace et {
namespace gui {


//====================
// RML File Interface
//====================


// static
Rml::FileHandle const RmlFileInterface::s_InvalidFileHandle = 0u;


//------------------------------------
// RmlFileInterface::Open
//
Rml::FileHandle RmlFileInterface::Open(Rml::String const& path)
{
	core::HashString const assetId(path.c_str());

	AssetPtr<GuiDocument> guiDocument = core::ResourceManager::Instance()->GetAssetData<GuiDocument>(assetId);
	if (guiDocument == nullptr)
	{
		return s_InvalidFileHandle;
	}

	std::pair<T_Files::iterator, bool> res = m_Files.emplace(++m_LastFileHandle, guiDocument);
	ET_ASSERT(res.second);

	return res.first->first;
}

//------------------------------------
// RmlFileInterface::Close
//
void RmlFileInterface::Close(Rml::FileHandle file)
{
	size_t const erased = m_Files.erase(file);
	ET_ASSERT(erased == 1u);
}

//------------------------------------
// RmlFileInterface::Read
//
size_t RmlFileInterface::Read(void* buffer, size_t size, Rml::FileHandle file)
{
	File& fileImpl = m_Files[file];
	size_t const bytesToRead = std::min(size, fileImpl.m_Asset->GetLength() - fileImpl.m_ReadPos);

	memcpy(buffer, reinterpret_cast<void const*>(fileImpl.m_Asset->GetText() + fileImpl.m_ReadPos), bytesToRead);
	fileImpl.m_ReadPos += bytesToRead;
	return bytesToRead;
}

//------------------------------------
// RmlFileInterface::Seek
//
bool RmlFileInterface::Seek(Rml::FileHandle file, long offset, int origin)
{
	File& fileImpl = m_Files[file];

	long pos;
	switch (origin)
	{
	case SEEK_SET:
		pos = 0;
		break;
	case SEEK_CUR:
		pos = static_cast<long>(fileImpl.m_ReadPos);
		break;
	case SEEK_END:
		pos = static_cast<long>(fileImpl.m_Asset->GetLength());
		break;
	}

	pos += offset;

	if ((pos < 0) || (pos > fileImpl.m_Asset->GetLength()))
	{
		return false;
	}

	fileImpl.m_ReadPos = static_cast<size_t>(pos);
	return true;
}

//------------------------------------
// RmlFileInterface::Tell
//
size_t RmlFileInterface::Tell(Rml::FileHandle file)
{
	File& fileImpl = m_Files[file];
	return fileImpl.m_ReadPos;
}


} // namespace gui
} // namespace et
