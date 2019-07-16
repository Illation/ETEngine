#pragma once
#include "Singleton.h"
#include "Time.h"


//-----------------------
// BaseContext
//
// Minimal context needed for core library features
//
struct BaseContext
{
	BaseContext() : time(new Time()) {}
	virtual ~BaseContext()
	{
		SafeDelete(time);
	}

	Time* time = nullptr;
};

//-----------------------
// Context manager
//
// Maintains the active context - scenes could have their own context, and when a scene gets activated we set the current context to the scenes context
//
class ContextManager : public Singleton<ContextManager>
{
private:
	// types
	//-----------
	friend class Singleton<ContextManager>;

	// ctor dtor
	//-----------
	ContextManager() = default;
	virtual ~ContextManager() = default;

	// utility
	//-----------
public:
	void SetActiveContext(BaseContext* context) { m_ActiveContext = context; }
	BaseContext* GetActiveContext() { return m_ActiveContext; }

	// Data
	////////
private:
	BaseContext* m_ActiveContext = nullptr;
};

