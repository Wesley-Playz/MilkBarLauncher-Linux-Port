#pragma once
#include <Windows.h>
#include "Vec3f.h"

using namespace DataTypes;

namespace DTO
{

	class DeathSwapDTO 
	{
	public:
		byte Phase = 0;
		Vec3f Position;
	};

}