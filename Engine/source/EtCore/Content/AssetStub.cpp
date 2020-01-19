#include "stdafx.h"

#include "AssetStub.h"

#include <EtCore/Reflection/Registration.h>
#include <EtCore/FileSystem/FileUtil.h>


//===================
// Stub Asset
//===================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_POLYMORPHIC_CLASS(StubAsset, "stub asset")
	END_REGISTER_POLYMORPHIC_CLASS(StubAsset, I_Asset);
}
DEFINE_FORCED_LINKING(StubAsset) // force the asset class to be linked as it is only used in reflection

//---------------------------------
// StubAsset::LoadFromMemory
//
// Load stub data from a file
//
bool StubAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	// Create data as a view of loaded memory
	m_Data = new StubData();
	m_Data->m_Text = reinterpret_cast<char const*>(data.data());
	m_Data->m_Length = data.size();

	// all done
	return true;
}

