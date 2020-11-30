#include "stdafx.h"

#include "AssetStub.h"

#include <EtCore/Reflection/Registration.h>
#include <EtCore/Content/AssetRegistration.h>
#include <EtCore/FileSystem/FileUtil.h>


namespace et {
namespace core {


//===================
// Stub Asset
//===================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS_ASSET(StubData, "stub data")
	END_REGISTER_CLASS(StubData);

	BEGIN_REGISTER_CLASS(StubAsset, "stub asset")
	END_REGISTER_CLASS_POLYMORPHIC(StubAsset, I_Asset);
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


} // namespace core
} // namespace et
