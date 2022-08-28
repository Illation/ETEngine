#pragma once
#include <EtCore/Content/Asset.h>
#include <EtCore/Util/LinkerUtils.h>


namespace et {
namespace gui {


//---------------------------------
// GuiDocument
//
// Data containing the UI definition for RmlUi
//
class GuiDocument final
{
public:
	GuiDocument() = default;
	~GuiDocument() = default;

	char const* GetText() const { return m_Text; }
	size_t GetLength() const { return m_Length; }

private:
	friend class GuiDocumentAsset;

	char const* m_Text;
	size_t m_Length = 0u;
};


//---------------------------------
// GuiDocumentAsset
//
// persistently loaded asset containing the data for a GuiDocumet
//
class GuiDocumentAsset final : public core::Asset<GuiDocument, true>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(core::Asset<GuiDocument, true>)
public:
	// Construct destruct
	//---------------------
	GuiDocumentAsset() : core::Asset<GuiDocument, true>() {}
	virtual ~GuiDocumentAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;


	// Data
	///////

	core::HashString m_DataModelId;
};


} // namespace gui
} // namespace et
