#pragma once

#include <rttr/type>

#include <EtCore/Content/Asset.h>
#include <EtCore/Helper/LinkerUtils.h>


//---------------------------------
// StubData
//
// Data to demonstrate the use of assets
//
class StubData final
{
public:
	StubData() = default;
	~StubData() = default;

	std::string const& GetText() const { return m_Text; }

private:
	friend class StubAsset;

	std::string m_Text;
};


//---------------------------------
// StubAsset
//
// simple asset to test the use of the resource manager
//
class StubAsset final : public Asset<StubData>
{
	DECLARE_FORCED_LINKING()
public:
	// Construct destruct
	//---------------------
	StubAsset() = default;
	virtual ~StubAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	// Utility
	//---------------------
private:

	RTTR_ENABLE(Asset<StubData>)
};

