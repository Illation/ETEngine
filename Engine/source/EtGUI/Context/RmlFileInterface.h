#pragma once
#include <RmlUi/Core/FileInterface.h>

#include <EtCore/Content/AssetPointer.h>

#include <EtGUI/Content/GuiDocument.h>


namespace et {
namespace gui {


//---------------------------------
// RmlFileInterface
//
// Implementation of RmlUi's file interface, to redirect file handling through the engine's resource manager
//
class RmlFileInterface final : public Rml::FileInterface
{
	//---------------------------------
	// File
	//
	// Internal file representation
	//
	struct File final
	{
		File() = default;
		File(AssetPtr<GuiDocument> const& asset) : m_Asset(asset) {}

		AssetPtr<GuiDocument> m_Asset;
		size_t m_ReadPos = 0u;
	};

	typedef std::unordered_map<Rml::FileHandle, File> T_Files;

	static Rml::FileHandle const s_InvalidFileHandle;

public:
	// construct destruct
	//--------------------
	RmlFileInterface() : Rml::FileInterface() {}
	~RmlFileInterface() = default;

	// interface implementation
	//--------------------------
	Rml::FileHandle Open(Rml::String const& path) override;
	void Close(Rml::FileHandle file) override;
	size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;
	bool Seek(Rml::FileHandle file, long offset, int origin) override;
	size_t Tell(Rml::FileHandle file) override;

	// Data
	///////

private:
	T_Files m_Files;
	Rml::FileHandle m_LastFileHandle = s_InvalidFileHandle;
};


} // namespace gui
} // namespace et

