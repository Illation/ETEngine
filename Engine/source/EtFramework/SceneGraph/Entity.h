#pragma once
#include <typeinfo>
#include <functional>


class AbstractComponent;
class AbstractScene;
class TransformComponent;


//---------------------
// Entity
//
// Base class for elements in a scene graph hierachy, can hold multiple components
//
class Entity final
{
	// definitsions
	//--------------
	friend class AbstractScene;

	// construct destruct
	//---------------------
public:
	Entity();
	virtual ~Entity();

	// Root
	//------
private:
	void RootInit();
	void RootDeinit();

	void RootUpdate();

	// functionality
	//---------------
public:
	void AddChild(Entity* const entity);
	void RemoveChild(Entity* const entity);

	void AddComponent(AbstractComponent* const component);
	void RemoveComponent(AbstractComponent* const component);

	void SetTag(std::string const& tag) { m_Tag = tag; }
	void SetName(std::string const& name);

	// accessors
	//------------
	void RecursiveAppendChildren(std::vector<Entity const*>& list) const;
	void RecursiveAppendChildren(std::vector<Entity*>& list);

	std::string const& GetTag() const { return m_Tag; }
	std::string const& GetName() const { return m_Name; }
	T_Hash GetId() const { return m_Id; } 

	TransformComponent* GetTransform() const { return m_Transform; }

	AbstractScene* GetScene();
	Entity* GetParent() const { return m_ParentEntity; }
	std::vector<Entity*> const& GetChildren() const { return m_Children; }

	template<class T> 
	bool HasComponent(bool searchChildren = false);

	template<class T> 
	T* GetComponent(bool searchChildren = false);

	template<class T> 
	T* GetChild();

	template<class T> 
	std::vector<T*> GetChildrenOfType();

	// Data
	///////

private:

	bool m_IsInitialized = false;
	bool m_IsActive = true;

	AbstractScene* m_ParentScene = nullptr;

	Entity* m_ParentEntity = nullptr;
	std::vector<Entity*> m_Children;

	TransformComponent* m_Transform = nullptr;
	std::vector<AbstractComponent*> m_Components;

	std::string m_Name;
	T_Hash m_Id = 0u;

	std::string m_Tag;
};


#include "Entity.inl"
