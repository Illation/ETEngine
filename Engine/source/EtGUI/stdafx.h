#pragma once

#pragma region
#include <EtRendering/stdafx.h>

#pragma endregion Includes

#pragma region

namespace et {
namespace gui {

typedef core::T_SlotId T_ContextId;
static constexpr T_ContextId INVALID_CONTEXT_ID = core::INVALID_SLOT_ID;

} // namespace gui
} // namespace et

namespace et {
	ET_DEFINE_TRACE_CTX(ET_CTX_GUI);
}

#pragma endregion Definitions