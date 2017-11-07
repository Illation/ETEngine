#pragma once
#include <functional>

class MulticastDelegate
{
public:
	void AddListener( std::function<void()> callback )
	{
		m_Listeners.push_back( callback );
	}
	void Broadcast()
	{
		for(auto listener : m_Listeners)
		{
			listener();
		}
	}
private:
	std::vector< std::function<void()> > m_Listeners;
};