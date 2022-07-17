#include "stdafx.h"

#include "TickManager.h"
#include "PerformanceInfo.h"

#include "Tickable.h"
#include "RealTimeTickTriggerer.h"
#include "DefaultTickTriggerer.h"
#include <EtCore/Input/InputManager.h>


namespace et {
namespace core {


//====================
// Tick Manager :: Tickable
//====================


//---------------------------------
// TickManager::operator=
//
// Copy assignment operator copies shallow
//
TickManager::Tickable& TickManager::Tickable::operator=(Tickable const& other)
{
	tickable = other.tickable; 
	priority = other.priority; 
	return *this;
}


//====================
// Tick Manager
//====================


//---------------------------------
// TickManager::TriggerRealTime
//
// Trigger a tick if this triggerer is currently registered and has already triggered a tick in the last cycle
//
void TickManager::TriggerRealTime(I_RealTimeTickTriggerer* const triggerer)
{
	std::vector<T_RealTimeTriggerer> triggerersCopy = m_RealTimeTriggerers; // ensure adding or removing during tick doesn't cause issues

	auto findResult = std::find_if(triggerersCopy.begin(), triggerersCopy.end(), [triggerer](T_RealTimeTriggerer const& rt)
	{
		return rt.first == triggerer;
	});

	// only trigger a tick if this triggerer is registered
	if (findResult != triggerersCopy.cend())
	{
		// If this triggerer has already been rendered but is calling again we should update everything
		if (findResult->second)
		{
			EndTick();

			for (T_RealTimeTriggerer& rt : m_RealTimeTriggerers)
			{
				rt.second = false;
			}

			Tick();
		}

		// we need to find this triggerer again in the original list to ensure the value is set in a useful way
		findResult = std::find_if(m_RealTimeTriggerers.begin(), m_RealTimeTriggerers.end(), [triggerer](T_RealTimeTriggerer const& rt)
			{
				return rt.first == triggerer;
			});

		ET_ASSERT(findResult != m_RealTimeTriggerers.cend());
		findResult->second = true;
	}
}

//---------------------------------
// TickManager::TriggerDefault
//
// Trigger a tick if this triggerer is currently registered and has already triggered a tick in the last cycle, and there are no real time triggerers
//
void TickManager::TriggerDefault(I_DefaultTickTriggerer* const triggerer)
{
	// if we are rendering in real time default triggerers shouldn't tick
	if (m_RealTimeTriggerers.size() > 0u)
	{
		return;
	}

	auto findResult = std::find_if(m_DefaultTriggerers.begin(), m_DefaultTriggerers.end(), [triggerer](T_DefaultTriggerer const& dt)
	{
		return dt.first == triggerer;
	});

	// only trigger a tick if this triggerer is registered
	if (findResult != m_DefaultTriggerers.cend())
	{
		// If this triggerer has already been rendered but is calling again we should update everything
		if (findResult->second)
		{
			EndTick();

			for (T_DefaultTriggerer& dt : m_DefaultTriggerers)
			{
				dt.second = false;
			}

			Tick();
		}

		findResult->second = true;
	}
}

//---------------------------------
// TickManager::RegisterRealTimeTriggerer
//
// Register a real time triggerer, disabling default tick triggering in the process
//
void TickManager::RegisterRealTimeTriggerer(I_RealTimeTickTriggerer* const triggerer)
{
	auto const findResult = std::find_if(m_RealTimeTriggerers.cbegin(), m_RealTimeTriggerers.cend(), [triggerer](T_RealTimeTriggerer const& rt)
	{
		return rt.first == triggerer;
	});

	if (findResult == m_RealTimeTriggerers.cend())
	{
		m_RealTimeTriggerers.emplace_back(triggerer, true); // by default we have already ticked so that this object can trigger a tick on init
	}
	else
	{
		ET_TRACE_W(ET_CTX_CORE, "TickManager::RegisterRealTimeTriggerer > RealTime triggerer already registered!");
	}
}

//---------------------------------
// TickManager::RegisterDefaultTriggerer
//
// Register a default triggerer which can tick if there are no real time triggerers active
//
void TickManager::RegisterDefaultTriggerer(I_DefaultTickTriggerer* const triggerer)
{
	auto const findResult = std::find_if(m_DefaultTriggerers.cbegin(), m_DefaultTriggerers.cend(), [triggerer](T_DefaultTriggerer const& dt)
	{
		return dt.first == triggerer;
	});

	if (findResult == m_DefaultTriggerers.cend())
	{
		m_DefaultTriggerers.emplace_back(triggerer, true); // by default we have already ticked so that this object can trigger a tick on init
	}
	else
	{
		ET_TRACE_W(ET_CTX_CORE, "TickManager::RegisterDefaultTriggerer > Default triggerer already registered!");
	}
}

//---------------------------------
// TickManager::RegisterTickable
//
// Add a triggerable object by inserting it into the list. objects of the highest priority get ticked first. objects of the same priority get ticked in 
//  - order of registration
//
void TickManager::RegisterTickable(I_Tickable* const tickableObject, uint32 const priority)
{
	// find the first tickable with a lower priority but also make sure our tickable isn't registered yet
	auto findResult = m_Tickables.end();
	for (auto it = m_Tickables.begin(); it != m_Tickables.end(); ++it)
	{
		if ((*it).tickable == tickableObject)
		{
			ET_TRACE_W(ET_CTX_CORE, "TickManager::RegisterTickable > object already registered!");
			return;
		}

		if (findResult == m_Tickables.cend() && (*it).priority < priority)
		{
			findResult = it;
		}
	}

	// add at end if there is nothing with a lower priority
	if (findResult == m_Tickables.cend())
	{
		m_Tickables.emplace_back(tickableObject, priority);
	}
	else
	{
		m_Tickables.insert(findResult, Tickable(tickableObject, priority));
	}
}

//---------------------------------
// TickManager::UnregisterRealTimeTriggerer
//
// Remove a realtime triggerer from the list. Doesn't log if it doesn't do anything due to not being in the list
//
void TickManager::UnregisterRealTimeTriggerer(I_RealTimeTickTriggerer* const triggerer)
{
	auto findResult = std::find_if(m_RealTimeTriggerers.begin(), m_RealTimeTriggerers.end(), [triggerer](T_RealTimeTriggerer const& rt)
	{
		return rt.first == triggerer;
	});

	if (findResult != m_RealTimeTriggerers.cend())
	{
		*findResult = std::move(m_RealTimeTriggerers.back());
		m_RealTimeTriggerers.pop_back();
	}
}

//---------------------------------
// TickManager::UnregisterRealTimeTriggerer
//
// Remove a default triggerer from the list. Doesn't log if it doesn't do anything due to not being in the list
//
void TickManager::UnregisterDefaultTriggerer(I_DefaultTickTriggerer* const triggerer)
{
	auto findResult = std::find_if(m_DefaultTriggerers.begin(), m_DefaultTriggerers.end(), [triggerer](T_DefaultTriggerer const& dt)
	{
		return dt.first == triggerer;
	});

	if (findResult != m_DefaultTriggerers.cend())
	{
		*findResult = std::move(m_DefaultTriggerers.back());
		m_DefaultTriggerers.pop_back();
	}
}

//---------------------------------
// TickManager::UnregisterTickable
//
// Stop ticking this object. Can be a slow function due to erasing needing to move all objects in the tickable list that come after
//
void TickManager::UnregisterTickable(I_Tickable* const tickableObject)
{
	auto findResult = std::find_if(m_Tickables.begin(), m_Tickables.end(), [tickableObject](Tickable const& tickable)
	{
		return tickable.tickable == tickableObject;
	});

	if (findResult == m_Tickables.cend())
	{
		ET_TRACE_W(ET_CTX_CORE, "TickManager::UnregisterTickable > attempted to remove a non registered object");
		return;
	}

	// no remove and swap possible because the order of tickables matters
	m_Tickables.erase(findResult);
}

//---------------------------------
// TickManager::Tick
//
// Start time and performance monitoring, then tick all tickable objects in order of their priority, and finally update the state of input
//
void TickManager::Tick()
{
	BaseContext const* const context = ContextManager::GetInstance()->GetActiveContext();
	if (context != nullptr)
	{
		// start new frame timer and performance
		context->time->Update();

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
		PerformanceInfo::GetInstance()->StartFrameTimer();
#endif
	}

	// tick all objects
	for (Tickable& tickableObject : m_Tickables)
	{
		tickableObject.tickable->OnTick();
	}

	// Update keystates
	InputManager::GetInstance()->Update();
}

//---------------------------------
// TickManager::EndTick
//
// At the end of the tick stop performance tracking
//
void TickManager::EndTick()
{
	// update performance info
#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	PerformanceInfo::GetInstance()->Update();
#endif
}


} // namespace core
} // namespace et
