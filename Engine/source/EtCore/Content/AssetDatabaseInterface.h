#pragma once


namespace et { namespace core {
	class I_Asset;
} }


namespace et {
namespace core {


//---------------------------------
// Asset Database Interface
//
class I_AssetDatabase
{
public:
	typedef std::function<void(I_Asset*)> T_AssetFunc;

	virtual ~I_AssetDatabase() = default;

	virtual void IterateAllAssets(T_AssetFunc const& func) = 0;
};


} // namespace core
} // namespace et
