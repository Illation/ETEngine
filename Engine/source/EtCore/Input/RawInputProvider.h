#pragma once
#include "RawInputListener.h"


namespace et {
namespace core {


//---------------------------------
// RawInputProvider
//
// Class that generates input events for input listeners
//
class RawInputProvider
{
	// definitions
	//-------------
public:
	typedef std::function<bool(I_RawInputListener&)> T_EventFn; // return true if consumed

	// construct destruct
	//--------------------
	virtual ~RawInputProvider() = default;

	// functionality
	//---------------
	void RegisterListener(Ptr<I_RawInputListener> const listener);
	void UnregisterListener(I_RawInputListener const* const listener);

	bool IterateListeners(T_EventFn const& callback);

	// Data
	///////

private:
	std::vector<Ptr<I_RawInputListener>> m_Listeners;
};


} // namespace core
} // namespace et

