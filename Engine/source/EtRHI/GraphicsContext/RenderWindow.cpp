#include "stdafx.h"
#include "RenderWindow.h"


namespace et {
namespace rhi {


//===============
// Render Window
//===============


//---------------------------------
// RenderWindow::d-tor
//
// Ensure all listeners are informed and can react to the window closing
//
RenderWindow::~RenderWindow()
{
	for (I_Listener* const listener : m_Listeners)
	{
		listener->OnWindowDestroyed(this);
	}
}

//---------------------------------
// RenderWindow::RegisterListener
//
void RenderWindow::RegisterListener(I_Listener* const listener)
{
	ET_ASSERT(std::find(m_Listeners.cbegin(), m_Listeners.cend(), listener) == m_Listeners.cend());
	m_Listeners.push_back(listener);
}


} // namespace rhi
} // namespace et

