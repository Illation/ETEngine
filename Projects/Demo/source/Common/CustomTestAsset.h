#pragma once

#include <EtCore/Content/Asset.h>



namespace et {
namespace demo {


//---------------------------------
// TestData
//
// Basically StubData
//
class TestData final
{
public:
	TestData() = default;
	~TestData() = default;

	char const* GetText() const { return m_Text; }
	size_t GetLength() const { return m_Length; }

private:
	friend class TestAsset;

	char const* m_Text;
	size_t m_Length = 0u;
};


//---------------------------------
// TestAsset
//
// basically a StubAsset
//
class TestAsset final : public core::Asset<TestData, true>
{
	RTTR_ENABLE(core::Asset<TestData, true>)
	DECLARE_FORCED_LINKING()
public:
	// Construct destruct
	//---------------------
	TestAsset() : core::Asset<TestData, true>() {}
	virtual ~TestAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;
};


} // namespace demo
} // namespace et
