#pragma once
#include <io.h>

#include "TraceFwd.h"

#include <EtCore/Hashing/Hash.h>
#include <EtCore/Memory/Create.h>
#include <EtCore/Memory/RefPointer.h>
#include <EtCore/Memory/Pointer.h>


namespace et {
namespace core {


//--------------
// E_TraceLevel
//
typedef uint8 T_TraceLevel;
enum E_TraceLevel : T_TraceLevel
{
	TL_Verbose	= 1u << 0,
	TL_Info		= 1u << 1,
	TL_Warning	= 1u << 2,
	TL_Error	= 1u << 3,
	TL_Fatal	= 1u << 5
};

typedef T_Hash T_TraceContext;


//--------------
// TraceService
//
// Processes trace messages by sending them to a trace server or writing them to a file
//
class TraceService final
{
	// definitions
	//-------------
private:
	friend class RefPtr<TraceService>;
	friend class Create<TraceService>;

	static RefPtr<TraceService> s_Instance;

public:

	//-------------------
	// ContextContainer
	//
	struct ContextContainer
	{
		T_TraceContext RegisterContext(std::string const& name);
		std::string const& GetContextName(T_TraceContext const hash);

	private:
		std::unordered_map<T_TraceContext, std::string> m_Contexts;
	};

	// singleton access
	//------------------
	static RefPtr<TraceService> Instance() { return s_Instance; }
	static void Initialize();
	static void Destroy();
	static bool IsInitialized();

	static ContextContainer& GetContextContainer();

	// construct destruct
	//--------------------
private:
	TraceService();
	~TraceService();

	// functionality
	//---------------
public:
	void Trace(T_TraceContext const context, E_TraceLevel const level, bool const timestamp, std::string const& msg);

	void StartFileLogging(std::string const& fileName);
	void StopFileLogging();

	void EnableDate(bool const enabled) { m_AddDate = enabled; }


	// Data
	///////

private:

	Ptr<std::ostream> m_OutStream;
#ifdef ET_PLATFORM_WIN
	HANDLE m_ConsoleHandle;
#endif

	UniquePtr<std::ofstream> m_FileStream;

	bool m_AddDate;
};


} // namespace core
} // namespace et


// Macro for declaring a new trace context
#define ET_DEFINE_TRACE_CTX(ctx)\
struct ctx final\
{\
	static et::core::T_TraceContext const s_Id;\
}

// should be registered once in some source file - watch out for linker optimizing this away
#define ET_REGISTER_TRACE_CTX(ctx) et::core::T_TraceContext const ctx::s_Id(et::core::TraceService::GetContextContainer().RegisterContext(#ctx))



// we will use this to declare the trace context for the core library
namespace et {
	ET_DEFINE_TRACE_CTX(ET_CTX_CORE);
}
