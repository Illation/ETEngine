#pragma once
#include "AssetPointer.h"


namespace et {
namespace core {


class I_AssetDatabase;


//---------------------------------
// ResourceManager
//
// Class that manages the lifetime of assets
//
class ResourceManager 
{
	// Definitions
	//---------------------
private:
	static UniquePtr<ResourceManager> s_Instance;
	friend class ResourceManager;
	friend class I_AssetPtr;

protected:
	typedef std::function<I_Asset*(HashString const)> T_ReferenceAssetGetter;

	// Singleton
	//---------------------
public:
	static void SetInstance(UniquePtr<ResourceManager>&& instance);
	static void DestroyInstance();

	static ResourceManager* Instance() { return s_Instance.Get(); }

public:
	// Construct destruct
	//---------------------
	ResourceManager() = default;
	virtual ~ResourceManager() = default;

	// Protect from copy construction
private:
	ResourceManager(const ResourceManager& t);
	ResourceManager& operator=(const ResourceManager& t);

public:
	// Accessors
	//---------------------
	bool IsUnloadDeferred() const { return m_DeferUnloadToFlush; }

	// Managing assets
	//---------------------
	template <class T_DataType>
	AssetPtr<T_DataType> GetAssetData(HashString const assetId, bool const reportWarnings = true);

	// utility
	//---------------------
protected:
	void SetAssetReferences(I_AssetDatabase* const db, T_ReferenceAssetGetter const& fnc) const;

	// Interface
	//---------------------
	virtual void Init() = 0;
	virtual void Deinit() = 0;

	virtual void LoadAsset(I_Asset* const asset); // used by asset pointer
	virtual void UnloadAsset(I_Asset* const asset);

public:
	virtual bool GetLoadData(I_Asset const* const asset, std::vector<uint8>& outData) const = 0;

	virtual void Flush() = 0; 

protected:
	virtual I_Asset* GetAssetInternal(HashString const assetId, rttr::type const type, bool const reportErrors) = 0;

	// Data
	///////

	bool m_DeferUnloadToFlush = false;
};


} // namespace core
} // namespace et


#include "ResourceManager.inl"
