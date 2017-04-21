#pragma once
#include <typeinfo>
#include <unordered_map>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>

class AbstractLoader
{
public:
	AbstractLoader(void){}
	virtual ~AbstractLoader(void){}

	virtual const std::type_info& GetType() const = 0;
	virtual void Unload() = 0;

private:
	AbstractLoader( const AbstractLoader &obj);
	AbstractLoader& operator=( const AbstractLoader& obj );
};

template <class T>
class ContentLoader : public AbstractLoader
{
public:
	ContentLoader(void)
	{
		++m_loaderReferences;
	}

	~ContentLoader(void)
	{
		
	}

	virtual const std::type_info& GetType() const { return typeid(T); }

	T* GetContent(const std::string& assetFile)
	{
		for(std::pair<std::string, T*> kvp:m_contentReferences)
		{
			if(kvp.first.compare(assetFile)==0)
				return kvp.second;
		}

		//Does File Exists?
		//struct _stat buff;
		//int result = -1;
		//result = _wstat(assetFile.c_str(), &buff);
		//if(result != 0)
		//{
		//	std::cout<<"ContentManager> File not found!\nPath: " << assetFile << std::endl;
		//}

		T* content = LoadContent(assetFile);
		if(content!=nullptr)m_contentReferences.insert(std::pair<std::string,T*>(assetFile, content));

		return content;
	}

	virtual void Unload()
	{
		--m_loaderReferences;

		if(m_loaderReferences<=0)
		{
			for(std::pair<std::string,T*> kvp:m_contentReferences)
			{
				Destroy(kvp.second);
			}

			m_contentReferences.clear();
		}
	}

protected:
	virtual T* LoadContent(const std::string& assetFile) = 0;
	virtual void Destroy(T* objToDestroy) = 0;

private:
	static std::unordered_map<std::string, T*> m_contentReferences;
	static int m_loaderReferences;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	ContentLoader( const ContentLoader &obj);
	ContentLoader& operator=( const ContentLoader& obj );
};

template<class T>
std::unordered_map<std::string, T*> ContentLoader<T>::m_contentReferences = std::unordered_map<std::string, T*>();

template<class T>
int ContentLoader<T>::m_loaderReferences = 0;

