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
// TraceService::AddHandler
//
// Add a message handler if it can initialize, otherwise return false
//
template <typename THandler, typename... Args>
bool TraceService::AddHandler(Args&&... args)
{
	ET_ASSERT(!HasHandler<THandler>());
	UniquePtr<I_TraceHandler> handler = Create<THandler>(std::forward<Args>(args)...);

	if (handler->Initialize())
	{
		m_Handlers.push_back(std::move(handler));
		return true;
	}

	return false;
}

//-----------------------------
// TraceService::RemoveHandler
//
template <typename THandler>
void TraceService::RemoveHandler()
{
	T_Handlers::const_iterator const foundHandlerIt = GetHandlerIt(rttr::get_type<THandler>());
	if (foundHandlerIt != m_Handlers.cend())
	{
		m_Handlers.erase(foundHandlerIt);
	}
}

//----------------------------------
// TraceService::GetHandlerIt
//
TraceService::T_Handlers::const_iterator TraceService::GetHandlerIt(rttr::type const handlerType) const
{
	return std::find_if(m_Handlers.cbegin(), m_Handlers.cend(), [handlerType](UniquePtr<I_TraceHandler> const& handler)
		{
			return (rttr::type::get(handler.Get()) == handlerType);
		});
}


} // namespace core
} // namespace et
