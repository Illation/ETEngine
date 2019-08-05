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

	// Definitions
	//---------------------

	friend class ResourceManager; 
	friend class I_AssetPtr;

	struct Reference final
	{
	public:
		void Init();

		std::string const& GetName() const { return m_Name; }
		void SetName(std::string const& val) { m_Name = val; }

		bool IsPersistent() const { return m_IsPersistent; }
		void SetPersistent(bool const val) { m_IsPersistent = val; }

		I_Asset* GetAsset() { return m_Asset; }

	private:
		// Data
		///////

		// reflected
		std::string m_Name;
		bool m_IsPersistent; // can the reference be unloaded after the asset was loaded

		// derived
		I_Asset* m_Asset;

		RTTR_ENABLE()
	};

	// Construct destruct
	//---------------------
	I_Asset() = default;
	I_Asset(bool const isPersistent) : m_IsPersistent(isPersistent) {}
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

	std::vector<Reference> const& GetReferences() const { return m_References; }
	void SetReferences(std::vector<Reference> const& val) { m_References = val; }

	T_Hash GetId() const { return m_Id; }
	T_Hash GetPackageId() const { return m_PackageId; }
	T_Hash GetPackageEntryId() const { return m_PackageEntryId; }

	uint32 GetRefCount() const { return m_RefCount; }

	void Load();

protected:
	// Data
	///////

	// reflected
	std::string m_Name;
	std::string m_Path;
	std::string m_PackageName; // an empty package implies that this is a compiled asset
	std::vector<Reference> m_References; // list of other assets this asset depends on

	// derived
	T_Hash m_Id;
	T_Hash m_PackageId;
	T_Hash m_PackageEntryId;

	uint32 m_RefCount;

	bool m_IsPersistent = false; // can the load data be unloaded after the asset was loaded
	std::vector<uint8> m_LoadData;

	RTTR_ENABLE()
};

//---------------------------------
// RawAsset
//
// Templated asset class with data without const template info
//
template <class T_DataType>
class RawAsset : public I_Asset
{
public:
	// Construct destruct
	//---------------------
	RawAsset() : I_Asset() {}
	RawAsset(bool const isPersistent) : I_Asset(isPersistent) {}
	virtual ~RawAsset() = default;

	// Interface
	//---------------------
	virtual bool LoadFromMemory(std::vector<uint8> const& data) { UNUSED(data); return false; }

	// Utility
	//---------------------
	std::type_info const& GetType() const override { return typeid(T_DataType); }
	bool IsLoaded() const override { return m_Data != nullptr; }
	void Unload() override;

	T_DataType const* GetData() const { return m_Data; }

protected:
	// Data
	///////

	T_DataType* m_Data = nullptr;

	RTTR_ENABLE(I_Asset)
};

//---------------------------------
// Asset
//
// Templated asset class with data and const template info
//
template <class T_DataType, bool T_IsPersistent>
class Asset : public RawAsset<T_DataType>
{
public:
	Asset() : RawAsset<T_DataType>(T_IsPersistent) {}
	virtual ~Asset() = default;

	// Interface
	//---------------------
	virtual bool LoadFromMemory(std::vector<uint8> const& data) override { UNUSED(data); return false; }

protected:
	RTTR_ENABLE(RawAsset<T_DataType>)
};

#include "Asset.inl"