#include "stdafx.h"

#include "AssetStub.h"

#include <rttr/registration>
#include <rttr/detail/policies/ctor_policies.h>

#include <EtCore/FileSystem/FileUtil.h>


//===================
// Stub Asset
//===================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<StubAsset>("stub asset")
		.constructor<StubAsset const&>()
		.constructor<>()(rttr::detail::as_object());
	rttr::type::register_converter_func([](StubAsset& stub, bool& ok) -> I_Asset*
	{
		ok = true;
		return new StubAsset(stub);
	});
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

