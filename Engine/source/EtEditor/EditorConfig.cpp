#include <Engine/stdafx.h>
#include "EditorConfig.h"

#include <EtCore/Reflection/Serialization.h>

#include <rttr/registration>


//======================
// Editor Config
//======================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<EditorConfig::DirPointers>("dir pointers")
		.property("project dir path", &EditorConfig::DirPointers::m_ProjectPath)
		.property("engine dir path", &EditorConfig::DirPointers::m_EnginePath)
		;
}


//---------------------------------
// EditorConfig::Initialize
//
// Load from JSON and generate derived data
//
void EditorConfig::Initialize()
{
	// try deserializing the user directory path
	if (!(serialization::DeserializeFromFile(s_PointerPath, m_DirPointers)))
	{
		LOG("EditorConfig::Initialize > unable to deserialize directory pointers!", LogLevel::Error);
	}
}
