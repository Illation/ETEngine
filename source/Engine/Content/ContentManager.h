#pragma once
#include <vector>
#include <typeinfo>
#include "ContentLoader.h"

class ContentManager
{
public:
	static void Initialize();
	static void AddLoader(AbstractLoader* loader);

	template<class T> 
	static T* Load(const std::string& assetFile)
	{
		const std::type_info& ti = typeid(T);
		for(AbstractLoader* loader:m_Loaders)
		{
			const std::type_info& loadertype = loader->GetType();
			if(loadertype == ti)
			{
				return (static_cast<ContentLoader<T>*>(loader))->GetContent(assetFile);
			}
		}

		return nullptr;
	}

	template<class T>
	static T* Reload(const std::string& assetFile)
	{
		const std::type_info& ti = typeid(T);
		for (AbstractLoader* loader : m_Loaders)
		{
			const std::type_info& loadertype = loader->GetType();
			if (loadertype == ti)
			{
				return (static_cast<ContentLoader<T>*>(loader))->ReloadContent(assetFile);
			}
		}

		return nullptr;
	}

	template<class T, class DataType>
	static T* GetLoader()
	{
		const std::type_info& ti = typeid(DataType);
		for (AbstractLoader* loader : m_Loaders)
		{
			const std::type_info& loadertype = loader->GetType();
			if (loadertype == ti)
			{
				return (static_cast<T*>(loader));
			}
		}
		return nullptr;
	}

	static void Release();

private:
	ContentManager();
	~ContentManager(void);

	static std::vector<AbstractLoader*> m_Loaders;
	static bool m_IsInitialized;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	ContentManager(const ContentManager& t);
	ContentManager& operator=(const ContentManager& t);
};

