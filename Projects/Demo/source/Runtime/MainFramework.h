#pragma once

#include <EtRuntime/AbstractFramework.h>


namespace et {
namespace demo {


//--------------------------
// MainFramework
//
// User facing wrapper around the engine
//
class MainFramework final : public rt::AbstractFramework
{
	// construct destruct
	//--------------------
public:
	MainFramework() : AbstractFramework() {} // initializes the engine
	~MainFramework() = default;

	// framework interface
	//---------------------
private:
	void OnSystemInit() override;
	void OnInit() override;
	void OnTick() override;

	// utility
	//---------
	void OnSceneActivated();


	// Data
	///////

	size_t m_CurrentScene = 0u;
};


} // namespace demo
} // namespace et
