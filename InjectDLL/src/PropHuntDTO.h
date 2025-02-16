#pragma once
#include <Windows.h>
#include "Vec3f.h"

using namespace DataTypes;

namespace DTO
{
	class PropHuntDTO
	{
	public:
		bool IsPlaying;
		byte Phase;
		Vec3f StartingPosition;
		bool IsHunter;
	};
}