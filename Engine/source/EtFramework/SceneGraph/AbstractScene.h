#pragma once
#include <EtCore/Helper/Hash.h>


//forward declaration
class Entity;
class CameraComponent;
struct SceneContext;
class SceneManager;
class AudioListenerComponent;
class PhysicsWorld;


//---------------------------
// AbstractScene
//
// Root scene - currently works through inheritance
//
class AbstractScene
{
	// definitions
	//--------------
	friend class SceneManager;

	// construct destruct
	//---------------------
public:
	AbstractScene(std::string const& name);
	virtual ~AbstractScene();

	// interface
	//-------------
protected:
	virtual void Init() = 0;

	virtual void Update() = 0;

	virtual void OnActivated() {}
	virtual void OnDeactivated() {}

	// root
	//------
private:
	void RootActivate();
	void RootDeactivate();

	void RootInit();
	void RootDeinit();

	void RootUpdate();

	// functionality
	//----------------
public:
	void AddEntity(Entity* pEntity);
	void RemoveEntity(Entity* pEntity, bool deleteEntity = true);

	// single items
	void SetActiveCamera(CameraComponent* pCamera);
	void SetAudioListener(AudioListenerComponent* val) { m_AudioListener = val; }

	void SetSkybox(T_Hash const assetId);
	void SetStarfield(T_Hash const assetId);

	// accessors
	//------------
	std::string const& GetName() const { return m_Name; }
	bool IsInitialized() const { return m_IsInitialized; }

	Entity* GetEntity(T_Hash const id) const;

	std::vector<Entity*> const& GetEntities() { return m_pEntityVec; }
	std::vector<Entity*> const& GetEntities() const { return m_pEntityVec; }

	PhysicsWorld* GetPhysicsWorld() const { return m_pPhysicsWorld; }
	AudioListenerComponent* GetAudioListener() const { return m_AudioListener; }
	CameraComponent const* GetActiveCamera() const;

	// utility
	//------------
private:
	void GetUniqueEntityName(std::string const& suggestion, std::string& uniqueName) const;


	// Data
	////////

	// ID
	std::string m_Name;

	// State
	bool m_IsInitialized = false;

	std::vector<Entity*> m_pEntityVec;
	PhysicsWorld* m_pPhysicsWorld = nullptr;

	// Single items
	SceneContext* m_SceneContext = nullptr;
	AudioListenerComponent* m_AudioListener = nullptr;
};

