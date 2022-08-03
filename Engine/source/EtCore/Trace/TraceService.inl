#pragma once


namespace et {
namespace core {


//===============
// Trace Service
//===============


//--------------------------
// TraceService::HasHandler
//
template <typename THandler>
bool TraceService::HasHandler() const
{
	return (GetHandlerIt(rttr::type::get<THandler>()) != m_Handlers.cend());
}

//--------------------------
// TraceService::HasHandler
//
template <typename THandler>
THandler* TraceService::GetHandler()
{
	T_Handlers::iterator const foundHandlerIt = GetHandlerIt(rttr::type::get<THandler>());
	if (foundHandlerIt == m_Handlers.cend())
	{
		return nullptr;
	}

	return static_cast<THandler*>((*foundHandlerIt).Get());
}

//--------------------------
// TraceService::AddHandler
//
// Add a message handler if it can initialize, otherwise return false
//
template <typename THandler, typename... Args>
THandler* TraceService::AddHandler(Args&&... args)
{
	ET_ASSERT(!HasHandler<THandler>());
	UniquePtr<I_TraceHandler> handler = Create<THandler>(std::forward<Args>(args)...);

	if (handler->Initialize())
	{
		m_Handlers.push_back(std::move(handler));
		return static_cast<THandler*>(m_Handlers.back().Get());
	}

	return nullptr;
}

//-----------------------------
// TraceService::RemoveHandler
//
template <typename THandler>
void TraceService::RemoveHandler()
{
	T_Handlers::iterator foundHandlerIt = GetHandlerIt(rttr::type::get<THandler>());
	if (foundHandlerIt != m_Handlers.cend())
	{
		*foundHandlerIt = nullptr;
		m_Handlers.erase(foundHandlerIt);
	}
}

//----------------------------------
// TraceService::GetHandlerIt
//
TraceService::T_Handlers::iterator TraceService::GetHandlerIt(rttr::type const handlerType) 
{
	return std::find_if(m_Handlers.begin(), m_Handlers.end(), [handlerType](UniquePtr<I_TraceHandler> const& handler)
		{
			return (rttr::type::get(*handler) == handlerType);
		});
}

//----------------------------------
// TraceService::GetHandlerIt
//
TraceService::T_Handlers::const_iterator TraceService::GetHandlerIt(rttr::type const handlerType) const
{
	return std::find_if(m_Handlers.cbegin(), m_Handlers.cend(), [handlerType](UniquePtr<I_TraceHandler> const& handler)
		{
			return (rttr::type::get(*handler) == handlerType);
		});
}


} // namespace core
} // namespace et
