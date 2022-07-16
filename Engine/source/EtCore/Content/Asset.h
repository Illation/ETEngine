#pragma once
#include <EtCore/Hashing/HashString.h>
#include <EtCore/Reflection/Registration.h>
#include <EtCore/Util/Assert.h>

#include <rttr/type>


namespace et {
	class I_AssetPtr;
	REGISTRATION_NS(core);

	namespace pl {
		class EditorAssetBase;
} }


namespace et {
namespace core {


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
	friend class pl::EditorAssetBase;

	struct Reference final
	{
	public:
		Reference(HashString const id);
		~Reference();

		HashString GetId() const { return m_Id; }

		I_AssetPtr* GetAsset() { return m_AssetPtr; }
		I_AssetPtr const* GetAsset() const { return m_AssetPtr; }

	private:
		friend class I_Asset;
		friend class ResourceManager;
		friend class pl::EditorAssetBase;

		void Ref();
		void Deref();

		// Data
		///////

		// reflected
		HashString m_Id;

		// derived
		I_Asset* m_Asset = nullptr; // pointer to the raw asset, shouldn't be directly used
		I_AssetPtr* m_AssetPtr = nullptr;
	};

	// Construct destruct
	//---------------------
	I_Asset() = default;
	I_Asset(bool const isPersistent) : m_IsPersistent(isPersistent) {}
	I_Asset(const I_Asset&) = default;
	virtual ~I_Asset();

	// Interface
	//---------------------
	virtual rttr::type GetType() const = 0;
	virtual bool IsLoaded() const = 0;
public:
	virtual bool LoadFromMemory(std::vector<uint8> const& data) = 0;
protected:
	virtual void UnloadInternal() {}

private:
	virtual void* GetRawData() = 0;
	virtual void SetRawData(void* const data) = 0;

public:

	// Utility
	//---------------------	
	std::string const& GetName() const { return m_Name; }
	void SetName(std::string const& val);

	std::string const& GetPath() const { return m_Path; }
	void SetPath(std::string const& val);

	std::vector<HashString> GetReferenceIds() const;
	void SetReferenceIds(std::vector<HashString> val);

	std::vector<Reference> const& GetReferences() const { return m_References; }

	HashString GetId() const { return m_Id; }
	HashString GetPackageId() const { return m_PackageId; }
	void SetPackageId(HashString const id) { m_PackageId = id; }
	HashString GetPackageEntryId() const { return m_PackageEntryId; }

	uint32 GetRefCount() const { return m_RefCount; }

	std::vector<uint8> const& GetLoadData() const { return m_LoadData; }
	std::vector<uint8>& GetLoadData() { return m_LoadData; }

	void Load();
	void Unload(bool const force = false);

protected:

	// Data
	///////

	// reflected
	std::string m_Name;
	std::string m_Path;
	HashString m_PackageId;
	std::vector<Reference> m_References; // list of other assets this asset depends on

	// derived
	HashString m_Id;
	HashString m_PackageEntryId;

	uint32 m_RefCount = 0u;

	bool m_IsPersistent = false; // can the load data be unloaded after the asset was loaded
	std::vector<uint8> m_LoadData;

	RTTR_ENABLE()
	REGISTRATION_FRIEND_NS(core)
};

//---------------------------------
// RawAsset
//
// Templated asset class with data without const template info
//
template <class T_DataType>
class RawAsset : public I_Asset
{
	friend class pl::EditorAssetBase;

public:
	// Construct destruct
	//---------------------
	RawAsset() : I_Asset() {}
	RawAsset(bool const isPersistent) : I_Asset(isPersistent) {}
	virtual ~RawAsset() = default;

	// Interface
	//---------------------
	virtual bool LoadFromMemory(std::vector<uint8> const& data) { ET_UNUSED(data); return false; }

	// Utility
	//---------------------
	rttr::type GetType() const override { return rttr::type::get<T_DataType>(); }
	bool IsLoaded() const override { return m_Data != nullptr; }
protected:
	void UnloadInternal() override;

private:
	void* GetRawData() override { return static_cast<void*>(m_Data); }
	void SetRawData(void* const data) override { m_Data = static_cast<T_DataType*>(data); }

public:
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
	virtual bool LoadFromMemory(std::vector<uint8> const& data) override { ET_UNUSED(data); return false; }

protected:
	RTTR_ENABLE(RawAsset<T_DataType>)
};


} // namespace core
} // namespace et


#include "Asset.inl"