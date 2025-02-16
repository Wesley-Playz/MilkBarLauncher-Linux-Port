#pragma once
#include <vector>

namespace DataTypes
{

	class Vec3f
	{
	public:

		Vec3f() { _x = 0; _y = 0; _z = 0; }
		Vec3f(float x, float y, float z) { _x = x; _y = y; _z = z; }
		Vec3f(float val[3]) { _x = val[0]; _y = val[1]; _z = val[2]; }
		Vec3f(std::vector<float> val[3]) { _x = val->at(0); _y = val->at(1); _z = val->at(2); }

		float x() { return _x; }
		float y() { return _y; }
		float z() { return _z; }

		void x(float x) { _x = x; }
		void y(float y) { _y = y; }
		void z(float z) { _z = z; }

		float& operator[] (size_t i)
		{
			switch (i) {
			case 0: return _x;
			case 1: return _y;
			case 2: return _z;
			default: throw "Valid indices: 0, 1, 2";
			}
		}

	private:
		float _x;
		float _y;
		float _z;

	};

}