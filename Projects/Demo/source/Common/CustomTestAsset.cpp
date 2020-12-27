#include "stdafx.h"
#include "CustomTestAsset.h"

#include <EtCore/Reflection/Registration.h>
#include <EtCore/Content/AssetRegistration.h>


namespace et {
namespace demo {


//===================
// Test Asset
//===================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS_ASSET(TestData, "test data")
	END_REGISTER_CLASS(TestData);

	BEGIN_REGISTER_CLASS(TestAsset, "test asset")
	END_REGISTER_CLASS_POLYMORPHIC(TestAsset, core::I_Asset);
}
DEFINE_FORCED_LINKING(TestAsset) // force the asset class to be linked as it is only used in reflection

//---------------------------------
// TestAsset::LoadFromMemory
//
bool TestAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	// Create data as a view of loaded memory
	m_Data = new TestData();
	m_Data->m_Text = reinterpret_cast<char const*>(data.data());
	m_Data->m_Length = data.size();

	// all done
	return true;
}


} // namespace demo
} // namespace et
