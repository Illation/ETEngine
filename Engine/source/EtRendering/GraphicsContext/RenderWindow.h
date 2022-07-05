#pragma once


namespace et {
namespace render {


class I_GraphicsContextApi;


//---------------------------------
// GraphicsContextParams
//
// Settings to create a Graphics Context with
//
struct GraphicsContextParams final
{
	int32 m_VersionMajor = -1;
	int32 m_VersionMinor = -1;
	bool m_IsForwardCompatible = true; // (core profile)
	bool m_UseES = false;

	bool m_UseDebugInfo = false;

	bool m_DoubleBuffer = true;
	bool m_DepthBuffer = true;
	bool m_StencilBuffer = false;
};

//---------------------------------
// RenderWindow
//
// Interface for a class that can create a graphics context, which can in turn be used to present render areas
//
class RenderWindow
{
public:

	//---------------------------------
	// I_Listener
	//
	// Listen to window events
	//
	class I_Listener
	{
	public:
		virtual void OnWindowDestroyed(RenderWindow const* const) = 0;
	};

	virtual ~RenderWindow();

	// interface
	//-----------
	virtual I_GraphicsContextApi* CreateContext(GraphicsContextParams const& params) = 0;
	virtual void SetCursorPos(ivec2 const pos) = 0;

	virtual ivec2 GetDimensions() const = 0;
	virtual bool HasFocus() const = 0;

	// functionality
	//---------------
	void RegisterListener(I_Listener* const listener);

	// Data
	///////
private:
	std::vector<I_Listener*> m_Listeners;
};


} // namespace render
} // namespace et
