#pragma once
#include <EtRendering/GraphicsContext/ViewportEvents.h>


// fwd
namespace Rml {
	class Context;
}


namespace et {
namespace gui {


//---------------------------------
// Context
//
// Context for RML UI documents
//  - for now all contexts are screen space and the same size as the viewport
//
class Context
{
public:
	// construct destruct
	//--------------------
	Context() = default;
	~Context();

	void Init(std::string const& name, ivec2 const dimensions);

	// functionality
	//---------------
	void SetActive(bool const isActive) { m_Active = isActive; }
	void SetDimensions(ivec2 const dimensions);

	// accessors
	//-----------
	bool IsActive() const { return m_Active; }

	// Data
	///////

private:
	Ptr<Rml::Context> m_Context;
	bool m_Active = true;
};


} // namespace gui
} // namespace et

