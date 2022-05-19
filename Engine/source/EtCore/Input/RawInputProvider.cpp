#include "stdafx.h"
#include "RawInputProvider.h"


namespace et {
namespace core {


//------------------------------------
// RawInputProvider::RegisterListener
//
void RawInputProvider::RegisterListener(Ptr<I_RawInputListener> const listener)
{
	ET_ASSERT(listener != nullptr);
	ET_ASSERT(std::find(m_Listeners.begin(), m_Listeners.end(), listener) == m_Listeners.cend());

	m_Listeners.insert(std::upper_bound(m_Listeners.begin(), m_Listeners.end(), listener, 
		[](Ptr<I_RawInputListener> const& lh, Ptr<I_RawInputListener> const& rh)
		{
			return (lh->GetPriority() > rh->GetPriority());
		}), listener);
}

//------------------------------------
// RawInputProvider::RegisterListener
//
void RawInputProvider::UnregisterListener(I_RawInputListener const* const listener)
{
	auto const foundIt = std::find_if(m_Listeners.begin(), m_Listeners.end(), [listener](Ptr<I_RawInputListener> const& lh)
		{
			return (lh.Get() == listener);
		});
	ET_ASSERT(foundIt != m_Listeners.cend());

	m_Listeners.erase(foundIt);
}

//------------------------------------
// RawInputProvider::IterateListeners
//
// Returns true if the event is consumed
//
bool RawInputProvider::IterateListeners(T_EventFn const& callback)
{
	for (Ptr<I_RawInputListener> const& listener : m_Listeners)
	{
		if (callback(*listener))
		{
			return true;
		}
	}

	return false;
}


} // namespace core
} // namespace et
