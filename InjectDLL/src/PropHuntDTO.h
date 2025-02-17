#pragma once
#include "Vec3f.h"
#include <cstdint>

using namespace DataTypes;

namespace DTO
{
	class PropHuntDTO
	{
	public:
		bool IsPlaying;
		uint8_t Phase;
		Vec3f StartingPosition;
		bool IsHunter;
	};
}