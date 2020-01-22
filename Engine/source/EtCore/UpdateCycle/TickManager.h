#pragma once
#include <EtCore/Util/Singleton.h>


// forward declarations
class I_Tickable;
class I_RealTimeTickTriggerer;
class I_DefaultTickTriggerer;


//---------------------------------
// TickManager
//
// Class that decides when to call a new tick to optimize for vsync or non realtime rendering
//
class TickManager : public Singleton<TickManager>
{
private:
	// Definitions
	//----------------
	friend class I_RealTimeTickTriggerer;
	friend class I_DefaultTickTriggerer;
	friend class I_Tickable;

	friend class Singleton<TickManager>;

	//---------------------------------
	// Tickable
	//
	// Pairs a tickable object with a priority for its tick order
	//
	struct Tickable
	{
		Tickable(I_Tickable* const lTickable, uint32 const lPriority) : tickable(lTickable), priority(lPriority) {}

		Tickable& operator=(Tickable const& other);

		I_Tickable* tickable;
		uint32 priority;
	};

	typedef std::pair<I_RealTimeTickTriggerer const*, bool> T_RealTimeTriggerer;
	typedef std::pair<I_DefaultTickTriggerer const*, bool> T_DefaultTriggerer;

	// Defualt constructor and destructor
	//----------------------------
	TickManager() = default;
	virtual ~TickManager() = default;

	// Triggering ticks
	//------------------
	void TriggerRealTime(I_RealTimeTickTriggerer* const triggerer);
	void TriggerDefault(I_DefaultTickTriggerer* const triggerer);

	// Registering stuff
	//------------------
	void RegisterRealTimeTriggerer(I_RealTimeTickTriggerer* const triggerer);
	void RegisterDefaultTriggerer(I_DefaultTickTriggerer* const triggerer);

	void RegisterTickable(I_Tickable* const tickableObject, uint32 const priority);

	// Unregistering stuff
	//----------------------
	void UnregisterRealTimeTriggerer(I_RealTimeTickTriggerer* const triggerer);
	void UnregisterDefaultTriggerer(I_DefaultTickTriggerer* const triggerer);

	void UnregisterTickable(I_Tickable* const tickableObject);

	// The actual tick
	//----------------------
	void Tick();
	void EndTick();

private:

	// Data
	///////

	std::vector<T_RealTimeTriggerer> m_RealTimeTriggerers;
	size_t m_RealTimeTicks = 0;
	std::vector<T_DefaultTriggerer> m_DefaultTriggerers;
	size_t m_DefaultTicks = 0;

	std::vector<Tickable> m_Tickables;
};

