// this code doesn't run but reflects the goal of the entity component system implementation

//==========================
// Components
//==========================

//---------------
// Transform
//
struct Transform
{
	ECS_DECLARE_COMPONENT
public:

	vec3 position;
	mat4 local;
	mat4 world;
}

REFLECT(Transform);
ECS_REGISTER_COMPONENT(Transform);

//---------------
// Rigid Body
//
struct RigidBody
{
	ECS_DECLARE_COMPONENT
public:

	vec3 velocity;
}

REFLECT(RigidBody);
ECS_REGISTER_COMPONENT(RigidBody);

//---------------
// other
//
struct Player;
struct AI;

ECS_REGISTER_COMPONENT(Player);
ECS_REGISTER_COMPONENT(AI);


//==========================
// Systems
//==========================

//--------------------
// Transform System
//
// hierachically set entity transforms
//
struct TransformSystemIt : public ComponentIterator
{
	WriteAccess<Transform> transform;
	ReadParent<Transform> parent;
}

REFLECT(TransformSystemIt);

//----------------
class TransformSystem : System<TransformSystemIt>
{
	TransformSystem() : System<TransformSystemIt>() {}

	void Process(ComponentView<TransformSystemIt>& view) const override
	{
		for(TransformSystemIt it : view)
		{
			it.transform->local = etm::translate(it.transform->position);

			if (it.parent.Exists())
			{
				it.transform->world = it.parent->world;
			}
			else
			{
				it.transform->world = it.transform->local;
			}
		}
	}
}

// just need type info
REFLECT(TransformSystem); 

//--------------------
// Phsyics System
//
// move entities according to their current velocity
//
struct PhysicsSystemIt : public ComponentIteratorExclude<Player, AI> // doesn't iterate for entities that are players or AI
{
	WriteAccess<Transform> transform;
	ReadAccess<RigidBody> rigidBody;
}

REFLECT(PhysicsSystemIt);

//----------------
class PhysicsSystem : System<PhysicsSystemIt>
{
	PhysicsSystem()
		: System<PhysicsSystemIt>()
	{
		DeclareDependents<TransformSystem>(); // runs before transform update
		DeclareDependencies<OtherSystemA, OtherSystemB>(); // needs data calculated from these other systems
	}

	void Process(ComponentView<PhysicsSystemIt>& view) const override
	{
		float const dt = TIME->GetDelta();	

		for(PhysicsSystemIt it : view)
		{
			it.transform->position += it.rigidBody->velocity * dt;
		}
	}
}

REFLECT(PhysicsSystem);


//==========================
// Create Entities / Init
//==========================

//----------------
void main()
{
	EcsController ecs;

	DeserializeScene("myLevel.json"_hash, ecs); // load entities

	// dynamically add more entities
	T_EntityId const myPlayer = ecs.AddEntity({ 
		framework::MakeRawComponent(Transform()), 
		framework::MakeRawComponent(RigidBody()),
		framework::MakeRawComponent(Player())});

	// ensure the specified systems run
	ecs.RegisterSystem<TransformSystem>();
	ecs.RegisterSystem<PhysicsSystem>();

	// game loop
	while (true)
	{
		ecs.Process();
	}
}
