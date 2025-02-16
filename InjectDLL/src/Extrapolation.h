#pragma once
#include "Vec3f.h"

using namespace DataTypes;

namespace Helper
{
	static class Extrapolation
	{
	public:
		static Vec3f Next(Vec3f currentPosition, Vec3f speed, float deltaTime)
		{
			return Vec3f(currentPosition.x() + speed.x() * deltaTime, currentPosition.y() + speed.y() * deltaTime, currentPosition.z() + speed.z() * deltaTime);
		}

		static Vec3f CalcSpeed(Vec3f lastPosition, Vec3f newPosition, float deltaTime)
		{
			return Vec3f((newPosition.x() - lastPosition.x()) / deltaTime, (newPosition.y() - lastPosition.y()) / deltaTime, (newPosition.z() - lastPosition.z()) / deltaTime);
		}
	};
}