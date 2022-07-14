#pragma once

#define NOMINMAX

//#define ET_GRAPHICS_API_DEBUG
//#define ET_GRAPHICS_API_VERBOSE

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