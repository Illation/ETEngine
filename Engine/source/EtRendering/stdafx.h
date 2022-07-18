#pragma once

#define NOMINMAX


#pragma region
//C RunTime Header Files
#include <wchar.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

#pragma endregion stl

#pragma region
#include <EtCore/stdafx.h>
#include <EtCore/Hashing/Hash.h>

#include <EtRendering/GraphicsContext/GraphicsContextApi.h>
#include <EtRendering/GraphicsContext/ContextHolder.h>
#include <EtRendering/GraphicsContext/Viewport.h>
#pragma endregion Includes


namespace et {
	ET_DEFINE_TRACE_CTX(ET_CTX_RENDER);
}

#define ET_CT_RHI_DEBUG ET_DISABLED
#define ET_CT_RHI_VERBOSE ET_DISABLED
