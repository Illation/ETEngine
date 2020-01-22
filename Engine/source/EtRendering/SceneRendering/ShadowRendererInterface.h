#pragma once


namespace et {
namespace render {


class Camera;
class I_Material;


//---------------------------------
// I_ShadowRenderer
//
// Interface for a class that can draw a shadow depth map
//
class I_ShadowRenderer
{
public:
	virtual ~I_ShadowRenderer() = default;

	virtual void DrawShadow(I_Material const* const nullMaterial) = 0;
	virtual Camera const& GetCamera() const = 0;
};


} // namespace render
} // namespace et
