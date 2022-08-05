#pragma once


namespace et {
namespace core {


//---------------------------------
// E_MouseButton
//
// Types of mouse buttons
//
enum class E_CursorShape : uint8 
{
	None,
	Arrow,
	IBeam,
	Crosshair,
	Hand,
	SizeWE,
	SizeNS
};

//---------------------------------
// I_CursorShapeManager
//
// Interface class that can reshape the cursor
//
class I_CursorShapeManager
{
public:
	virtual ~I_CursorShapeManager() = default;

	virtual bool SetCursorShape(E_CursorShape const shape) = 0; // should return true on success
};


} // namespace core
} // namespace et
