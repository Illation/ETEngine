#include "stdafx.h"
#include "RmlFileInterface.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtGUI/Content/GuiDocument.h>


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
	return 0u;
}

//------------------------------------
// RmlFileInterface::Seek
//
bool RmlFileInterface::Seek(Rml::FileHandle file, long offset, int origin)
{
	return false;
}

//------------------------------------
// RmlFileInterface::Tell
//
size_t RmlFileInterface::Tell(Rml::FileHandle file)
{
	return 0u;
}


} // namespace gui
} // namespace et
