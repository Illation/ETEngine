#pragma once

#include <rttr/type>

#include <EtCore/Content/Asset.h>
#include <EtCore/Helper/LinkerUtils.h>


//---------------------------------
// StubData
//
// Data to demonstrate the use of assets - acts as a string view into loaded data
//
class StubData final
{
public:
	StubData() = default;
	~StubData() = default;

	char const* GetText() const { return m_Text; }

private:
	friend class StubAsset;

	char const* m_Text;
	size_t m_Length = 0u;
};


//---------------------------------
// StubAsset
//
// simple asset to test the use of the resource manager - loaded data here is persistent
//
class StubAsset final : public Asset<StubData, true>
{
	DECLARE_FORCED_LINKING()
public:
	// Construct destruct
	//---------------------
	StubAsset() : Asset<StubData, true>() {}
	virtual ~StubAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	// Utility
	//---------------------
private:

	RTTR_ENABLE(Asset<StubData, true>)
};

