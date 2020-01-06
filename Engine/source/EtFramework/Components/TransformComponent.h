#pragma once
#include "AbstractComponent.h"

#include <EtCore/Containers/slot_map.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>


class TransformComponent final : public AbstractComponent
{
public:
	TransformComponent();
	~TransformComponent() = default;

	void Translate(float x, float y, float z);
	void Translate(const vec3& translation );
	void SetPosition(float x, float y, float z);
	void SetPosition(const vec3& position);

	void RotateEuler(float x, float y, float z);
	void RotateEuler(const vec3& eulerAngles );
	void Rotate(const quat& rotation);
	void SetRotation(const quat& rotation) { m_Rotation = rotation; }

	void Scale(float x, float y, float z);
	void Scale(const vec3& scale);

	const vec3& GetPosition() const { return m_Position; }
	const vec3& GetWorldPosition() const { return m_WorldPosition; }
	const vec3& GetScale() const { return m_Scale; }
	const vec3& GetWorldScale() const { return m_WorldScale; }
	const quat& GetRotation() const { return m_Rotation; }
	const vec3& GetEuler() const { return m_Rotation.ToEuler(); }
	const quat& GetWorldRotation() const { return m_WorldRotation; }
	const mat4& GetWorld() const { return m_World; }

	const vec3& GetForward() const { return m_Forward; }
	const vec3& GetUp() const { return m_Up; }
	const vec3& GetRight() const { return m_Right; }

	core::T_SlotId GetNodeId() const { return m_NodeId; }

protected:

	void Init() override;
	void Deinit() override;
	void Update() override;


	void UpdateTransforms();

private:
	enum TransformChanged {
		NONE = 0x00,
		TRANSLATION = 0x01,
		ROTATION = 0x02,
		SCALE = 0x04,
	};

	uint8 m_IsTransformChanged;

	vec3 m_Position = vec3(),
		m_WorldPosition = vec3(),
		m_Scale = vec3( 1 ),
		m_WorldScale = vec3( 1 ),
		m_Forward = vec3::FORWARD,
		m_Up = vec3::UP,
		m_Right = vec3::RIGHT;
	quat m_Rotation = quat(),
		m_WorldRotation = quat();
	mat4 m_World;

	core::T_SlotId m_NodeId;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	TransformComponent(const TransformComponent& obj);
	TransformComponent& operator=( const TransformComponent& obj ) { UNUSED( obj );  delete this; }
};


namespace framework {


//---------------------------------
// TransformComponent
//
// Component that describes a world translation / rotation / scale, and utility functions to change them - relates to the parent entities transform
//
class TransformComponent final 
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	friend class TransformSystem;

	typedef uint8 T_TransformChanged;
	enum E_TransformChanged : T_TransformChanged
	{
		None		= 1 << 0,
		Translation = 1 << 1,
		Rotation	= 1 << 2,
		Scale		= 1 << 3
	};

	// construct destruct
	//--------------------
public:
	TransformComponent() = default;
	~TransformComponent() = default;

	// modifiers
	//-----------
	void Translate(float x, float y, float z);
	void Translate(const vec3& translation );
	void SetPosition(float x, float y, float z);
	void SetPosition(const vec3& position);

	void RotateEuler(float x, float y, float z);
	void RotateEuler(const vec3& eulerAngles );
	void Rotate(const quat& rotation);
	void SetRotation(const quat& rotation) { m_Rotation = rotation; }

	void SetScale(float x, float y, float z);
	void SetScale(const vec3& scale);

	// accessors
	//-----------
	const vec3& GetPosition() const { return m_Position; }
	const vec3& GetWorldPosition() const { return m_WorldPosition; }

	const vec3& GetScale() const { return m_Scale; }
	const vec3& GetWorldScale() const { return m_WorldScale; }

	const quat& GetRotation() const { return m_Rotation; }
	const vec3& GetEuler() const { return m_Rotation.ToEuler(); }
	const quat& GetWorldRotation() const { return m_WorldRotation; }

	const mat4& GetWorld() const { return m_WorldTransform; }

	const vec3& GetForward() const { return m_Forward; }
	const vec3& GetUp() const { return m_Up; }
	const vec3& GetRight() const { return m_Right; }

	core::T_SlotId GetNodeId() const { return m_NodeId; }

	// Data
	///////

private:
	vec3 m_Position;
	vec3 m_WorldPosition;

	quat m_Rotation;
	quat m_WorldRotation;

	vec3 m_Scale = vec3(1.f);
	vec3 m_WorldScale = vec3(1.f);

	vec3 m_Forward = vec3::FORWARD;
	vec3 m_Up = vec3::UP;
	vec3 m_Right = vec3::RIGHT;

	mat4 m_WorldTransform;

	T_TransformChanged m_TransformChanged = E_TransformChanged::None;
	core::T_SlotId m_NodeId = core::INVALID_SLOT_ID;
};


//---------------------------------
// TransformComponentDesc
//
// Descriptor for serialization and deserialization of transform components
//
class TransformComponentDesc final : public ComponentDescriptor<TransformComponent>
{
	// definitions
	//-------------
	RTTR_ENABLE(ComponentDescriptor<TransformComponent>)

	// construct destruct
	//--------------------
public:
	TransformComponentDesc() : ComponentDescriptor<TransformComponent>() {}
	~TransformComponentDesc() = default;

	// ComponentDescriptor interface
	//-------------------------------
	TransformComponent* MakeData() override;

	// Data
	///////

	vec3 position;
	quat rotation;
	vec3 scale = vec3(1.f);
};


} // namespace framework

