#pragma once
#include "stdafx.hpp"

struct FILE_ACCESS_FLAGS 
{

    FILE_ACCESS_FLAGS()
        : Flags(0)
    {

    }

    enum FLAGS 
	{
		Create =    1 << 0,
		Exists =    1 << 1,
		Truncate =  1 << 2
	};

	void SetFlags( int32 flags ) 
	{
		Flags = flags;
	}

	bool GetFlag( FLAGS flag ) 
	{
		auto ret = Flags & flag;
		return ret != 0;
	}

private:
	int32 Flags;
};
