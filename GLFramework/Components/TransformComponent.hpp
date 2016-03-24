#pragma once
#include "AbstractComponent.hpp"
#include <glm\gtc\quaternion.hpp>
class TransformComponent : public AbstractComponent
{
public:
	TransformComponent();
	virtual ~TransformComponent();

	void Translate(float x, float y, float z);
	void Translate(const glm::vec3& position);

	void RotateEuler(float x, float y, float z);
	void RotateEuler(const glm::vec3& rotation);
	void Rotate(const glm::quat& rotation);

	void Scale(float x, float y, float z);
	void Scale(const glm::vec3& scale);

	const glm::vec3& GetPosition() const { return m_Position; }
	const glm::vec3& GetWorldPosition() const { return m_WorldPosition; }
	const glm::vec3& GetScale() const { return m_Scale; }
	const glm::vec3& GetWorldScale() const { return m_WorldScale; }
	const glm::quat& GetRotation() const { return m_Rotation; }
	const glm::quat& GetWorldRotation() const { return m_WorldRotation; }
	const glm::mat4& GetWorld() const { return m_World; }

	const glm::vec3& GetForward() const { return m_Forward; }
	const glm::vec3& GetUp() const { return m_Up; }
	const glm::vec3& GetRight() const { return m_Right; }

protected:

	virtual void Initialize();
	virtual void Update();
	virtual void Draw();
	virtual void DrawForward();


	void UpdateTransforms();

private:
	enum TransformChanged {
		NONE = 0x00,
		TRANSLATION = 0x01,
		ROTATION = 0x02,
		SCALE = 0x04,
	};

	unsigned char m_IsTransformChanged;

	glm::vec3 m_Position = glm::vec3(0, 0, 0),
		m_WorldPosition = glm::vec3(0, 0, 0),
		m_Scale = glm::vec3(1, 1, 1),
		m_WorldScale = glm::vec3(1, 1, 1),
		m_Forward = glm::vec3(0, 0, 1),
		m_Up = glm::vec3(0, 1, 0),
		m_Right = glm::vec3(1, 0, 0);
	glm::quat m_Rotation = glm::quat(0, 0, 0, 1),
		m_WorldRotation = glm::quat(0, 0, 0, 1);
	glm::mat4 m_World;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	TransformComponent(const TransformComponent& obj);
	TransformComponent& operator=(const TransformComponent& obj) { delete this; }
};

