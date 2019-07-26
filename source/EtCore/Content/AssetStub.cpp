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
	// Extract the shader text from binary data
	//------------------------
	std::string text = FileUtil::AsText(data);
	if (text.size() == 0)
	{
		LOG("StubAsset::LoadFromMemory > Empty stub text file!", Warning);
		return false;
	}

	// Create shader data
	m_Data = new StubData();
	m_Data->m_Text = std::move(text);

	// all done
	return true;
}

