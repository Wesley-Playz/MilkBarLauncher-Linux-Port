#pragma once

#include "LocalInstance.h"
#include "Player.h"

namespace Instances
{
	static std::map<int, MemoryAccess::Player*> PlayerList;
}