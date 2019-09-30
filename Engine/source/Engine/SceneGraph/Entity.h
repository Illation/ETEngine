#pragma once
#include <string>
#include <vector>
#include <typeinfo>
#include <functional>


class AbstractComponent;
class AbstractScene;
class TransformComponent;


class Entity
{
private:
	friend class AbstractScene;
	friend class SceneRenderer;

	// construct destruct
	//---------------------
public:
	Entity();
	virtual ~Entity();

	// functionality
	//---------------
	void AddChild(Entity* pEntity);
	void RemoveChild(Entity* pEntity);

	void AddComponent(AbstractComponent* pComp);
	void RemoveComponent(AbstractComponent* pComp);

	void SetTag(std::string const& tag) { m_Tag = tag; }
	void SetName(std::string const& name);

	void RecursiveAppendChildren(std::vector<Entity const*>& list) const;

	// accessors
	//------------
	std::string const& GetTag() const { return m_Tag; }
	std::string const& GetName() const { return m_Name; }
	T_Hash GetId() const { return m_Id; } 

	TransformComponent* GetTransform() const { return m_pTransform; }

	AbstractScene* GetScene();
	Entity* GetParent() const { return m_pParentEntity; }

	template<class T> 
	bool HasComponent(bool searchChildren = false);

	template<class T> 
	T* GetComponent(bool searchChildren = false);

	template<class T> 
	std::vector<T*> GetComponents(bool searchChildren = false);

	template<class T> 
	T* GetChild();

	template<class T> 
	std::vector<T*> GetChildren();

	// interface
	//------------
protected:
	virtual void Initialize() {}
	virtual void Start() {}
	virtual void Draw() {}
	virtual void DrawForward() {}
	virtual void DrawShadow() {}
	virtual void Update() {}

private:

	// utility
	//------------
	void RootInitialize();
	void RootStart();
	void RootDraw();
	void RootDrawForward();
	void RootDrawShadow();
	void RootUpdate();

	// Data
	///////

	std::vector<Entity*> m_pChildVec;
	std::vector<AbstractComponent*> m_pComponentVec;

	bool m_IsInitialized = false;
	bool m_IsActive = true;
	AbstractScene* m_pParentScene = nullptr;
	Entity* m_pParentEntity = nullptr;
	TransformComponent* m_pTransform = nullptr;
	std::string m_Tag;

	std::string m_Name;
	T_Hash m_Id = 0u;
};

#include "Entity.inl"
