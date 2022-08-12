#pragma once


namespace et {
namespace pl {


//-----------------------------
// Editor Asset Meta Interface
//
// Allow storing editor specific data that is not necessary for the cooker, like thumbnail, layout, import settings and asset specific settings
//
class I_EditorAssetMeta
{
	RTTR_ENABLE()
public:
	virtual ~I_EditorAssetMeta() = default;
	virtual rttr::type GetType() const = 0;
};


} // namespace pl
} // namespace et
