#pragma once
#include <wtypes.h>


namespace et {
namespace core {


void DisplayError(LPTSTR lpszFunction);
// Routine Description:
// Retrieve and output the system error message for the last-error code

void GetExecutablePathName(std::string& outPath);


} // namespace core
} // namespace et
