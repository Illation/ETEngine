#pragma once


namespace et {
namespace core {


//---------------------------------
// I_RealTimeTickTriggerer
//
// Interface for things that call a tick at a realtime interval, for instance synced to the screen refresh rate
//
class I_RealTimeTickTriggerer
{
protected:
	virtual ~I_RealTimeTickTriggerer();

	void TriggerTick();

	void RegisterAsTriggerer();
	void UnRegisterAsTriggerer();
};


} // namespace core
} // namespace et
