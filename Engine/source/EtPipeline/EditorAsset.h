#pragma once

#include <EtCore/Content/Asset.h>

#include "EditorAssetMeta.h"


namespace et {
namespace pl  {
	

//---------------------------
// Editor Asset Base
//
// Can contain data relevant to converting an editor asset to its runtime version
//
class EditorAssetBase
{
	RTTR_ENABLE()
public:
	virtual ~EditorAssetBase() = default;

	virtual rttr::type GetType() const = 0;
	virtual core::I_Asset* GetAsset() = 0;
	virtual core::I_Asset const* GetAsset() const = 0;

protected:
	virtual bool LoadFromMemory(std::vector<uint8> const& data) = 0;
	// virtual bool WriteToMemory(std::vector<uint8>& data) = 0;
	virtual void UnloadInternal() {}


public:
	void Load();
	void Unload(bool const force = false);
	// void Save
	// void Generate(build configuration)


	// Data
	///////

private:
	core::HashString m_Id;
	std::string m_FilePath;

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
	RTTR_ENABLE(EditorAssetBase)
public:
	EditorAsset() : EditorAssetBase() {}
	virtual ~EditorAsset() = default;

	rttr::type GetType() const override { return rttr::type::get<TDataType>(); }
	core::I_Asset* GetAsset() override { return m_Asset; }
	core::I_Asset const* GetAsset() const override { return m_Asset; }

	virtual bool LoadFromMemory(std::vector<uint8> const& data) { return m_Asset->LoadFromMemory(data); } // can be overridden


	// Data
	///////

private:

	core::RawAsset<TDataType>* m_Asset = nullptr;
	
	I_EditorAssetMeta* m_MetaData = nullptr; // can stay null
};


} // namespace pl
} // namespace et
