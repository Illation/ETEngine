#pragma once

#include <EtCore/Helper/Hash.h>

#include <rttr/type>


//---------------------------------
// I_Asset
//
// Base asset interface without asset data
//
class I_Asset
{
public:
	// Construct destruct
	//---------------------
	I_Asset() = default;
	I_Asset(const I_Asset&) = default;
	virtual ~I_Asset();

	// Interface
	//---------------------
	virtual std::type_info const& GetType() const = 0;
	virtual bool IsLoaded() const = 0;
	virtual bool LoadFromMemory(std::vector<uint8> const& data) = 0;
	virtual void Unload() {}

	// Utility
	//---------------------	
	std::string const& GetName() const { return m_Name; }
	void SetName(std::string const& val);

	std::string const& GetPath() const { return m_Path; }
	void SetPath(std::string const& val);

	std::string const& GetPackageName() const { return m_PackageName; }
	void SetPackageName(std::string const& val);

	T_Hash GetId() const { return m_Id; }
	T_Hash GetPackageId() const { return m_PackageId; }
	T_Hash GetPackageEntryId() const { return m_PackageEntryId; }

	void Load();

protected:
	// Data
	///////

	// reflected
	std::string m_Name;
	std::string m_Path;
	std::string m_PackageName; // an empty package implies that this is a compiled asset

	// derived
	T_Hash m_Id;
	T_Hash m_PackageId;
	T_Hash m_PackageEntryId;

	RTTR_ENABLE()
};

//---------------------------------
// Asset
//
// Templated asset class with data
//
template <class T>
class Asset : public I_Asset
{
public:
	// Construct destruct
	//---------------------
	Asset() = default;
	virtual ~Asset() = default;

	// Interface
	//---------------------
	virtual bool LoadFromMemory(std::vector<uint8> const& data) { UNUSED(data); return false; }

	// Utility
	//---------------------
	std::type_info const& GetType() const override { return typeid(T); }
	bool IsLoaded() const override { return m_Data != nullptr; }
	void Unload() override;

	T const* GetData() const { return m_Data; }

protected:
	// Data
	///////

	T* m_Data = nullptr;

	RTTR_ENABLE(I_Asset)
};

#include "Asset.inl"