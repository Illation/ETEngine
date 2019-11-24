#pragma once


class NullMaterial;
class Camera;


namespace render {


//---------------------------------
// I_ShadowRenderer
//
// Interface for a class that can draw a shadow depth map
//
class I_ShadowRenderer
{
public:
	virtual ~I_ShadowRenderer() = default;

	virtual void DrawShadow(NullMaterial* const nullMaterial) = 0;
	virtual Camera const& GetCamera() const = 0;
};


} // namespace render
