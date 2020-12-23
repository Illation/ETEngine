#pragma once

#include <EtCore/Content/Asset.h>

#include "EditorAssetMeta.h"

namespace et {
	REGISTRATION_NS(pl);
	namespace core {
		class File;
	}
}


namespace et {
namespace pl {
	

//---------------------------
// Editor Asset Base
//
// Can contain data relevant to converting an editor asset to its runtime version
//
class EditorAssetBase
{
	// definitions
	//-------------
	RTTR_ENABLE()
	REGISTRATION_FRIEND_NS(pl)

	// construct destruct
	//--------------------
public:
	virtual ~EditorAssetBase();

	void Init(core::File* const configFile);

	// accessors
	//-----------
	core::HashString GetId() const { return m_Id; }

	virtual rttr::type GetType() const = 0;
	virtual core::I_Asset* GetAsset() = 0;
	virtual core::I_Asset const* GetAsset() const = 0;

	// utility
	//---------
protected:
	virtual bool LoadFromMemory(std::vector<uint8> const& data) = 0;
	// virtual bool WriteToMemory(std::vector<uint8>& data) = 0;
	virtual void UnloadInternal() {}

	// functionality
	//---------------
public:
	void Load();
	void Unload(bool const force = false);
	// void Save
	// void Generate(build configuration)


	// Data
	///////

protected:
	core::HashString m_Id;
	core::File* m_File = nullptr;

	core::I_Asset* m_Asset = nullptr;

	I_EditorAssetMeta* m_MetaData = nullptr; // can stay null

	std::vector<EditorAssetBase*> m_ChildAssets;
};

//------------------------
// Templated Editor Asset
//
// Modifiable asset with editor meta data that can generate a runtime asset
//
template <class TDataType>
class EditorAsset : public EditorAssetBase
{
	// definitions
	RTTR_ENABLE(EditorAssetBase)

	// constuct destruct
	//-------------------
public:
	EditorAsset() : EditorAssetBase() {}
	virtual ~EditorAsset() = default;

	// implementation
	//----------------
	rttr::type GetType() const override { return rttr::type::get<TDataType>(); }
	core::I_Asset* GetAsset() override { return m_Asset; }
	core::I_Asset const* GetAsset() const override { return m_Asset; }

	// accessors
	//-----------
	core::RawAsset<TDataType>* GetRawAsset() { return static_cast<core::RawAsset<TDataType>*>(m_Asset); }
	core::RawAsset<TDataType> const* GetRawAsset() const { return static_cast<core::RawAsset<TDataType>*>(m_Asset); }

	// interface
	//-----------
	virtual bool LoadFromMemory(std::vector<uint8> const& data) { return GetRawAsset()->LoadFromMemory(data); } // can be overridden
};


} // namespace pl
} // namespace et
