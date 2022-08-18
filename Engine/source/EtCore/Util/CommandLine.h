#pragma once
#include <EtCore/Util/CommonMacros.h>

#include "CommandLineParser.h"


#ifdef ET_SHIPPING
#	define ET_CT_DBG_COMMANDLINE ET_DISABLED
#else
#	define ET_CT_DBG_COMMANDLINE ET_ENABLED
#endif // ET_SHIPPING



#define ET_REGISTER_COMMANDLINE_S(name, data, description, shorthand) \
et::core::CommandLineParser::RegisterHelper g_CmdRegisterHelper##name( data, #name, description, shorthand );

#define ET_REGISTER_COMMANDLINE(name, data, description) ET_REGISTER_COMMANDLINE_S(name, data, description, 0)


#if ET_CT_IS_ENABLED(ET_CT_DBG_COMMANDLINE)
#	define ET_REGISTER_COMMANDLINE_DBG(name, data, description) ET_REGISTER_COMMANDLINE(name, data, description)
#else
#	define ET_REGISTER_COMMANDLINE_DBG(name, data, description)
#endif 
