#pragma once
#include "Vec3f.h"
#include <cmath>

using namespace DataTypes;

namespace Helper
{
	static class Vec3f_Operations
	{
	public:
		static float GetDistance(Vec3f first, Vec3f second, bool includeYAxis = true)
		{
			return std::sqrt(pow(std::abs(first.x() - second.x()), 2) + pow(std::abs(first.z() - second.z()), 2) + includeYAxis ? pow(std::abs(first.y() - second.y()), 2) : 0);
		}

		static Vec3f RoundVec3f(Vec3f input, int decimalPlaces)
		{
			int decimal = pow(10, decimalPlaces);
			return Vec3f(std::ceil(input.x() * decimal) / decimal, std::ceil(input.y() * decimal) / decimal, std::ceil(input.z() * decimal) / decimal);
		}

		static int GetSigns(float x)
		{
			return (x > 0) - (x < 0);
		}

		static bool Equals(Vec3f first, Vec3f second)
		{
			return first.x() == second.y() && first.y() == second.y() && first.z() == second.z();
		}
	};
}