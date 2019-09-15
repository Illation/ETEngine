#pragma once

// these utility functions can be used in a project to force the compiler to link classes that are only accessed by reflection in library files 
//  - this avoids letting the linker optimize them away

#define DECLARE_FORCED_LINKING() public: \
	void ForceLink(); \
	private:

#define DEFINE_FORCED_LINKING(typeName) void typeName::ForceLink() {}

#define FORCE_LINKING(typeName) typeName forceLinkObject_##typeName; \
	forceLinkObject_##typeName.ForceLink();