#pragma once
#include <vector>

namespace DataTypes
{

	class Quaternion
	{
	public:

		Quaternion() { _q1 = 0; _q2 = 0; _q3 = 0; _q4 = 0; }
		Quaternion(float q1, float q2, float q3, float q4) { _q1 = q1; _q2 = q2; _q3 = q3; _q4 = q4; }
		Quaternion(float val[4]) { _q1 = val[0]; _q2 = val[1]; _q3 = val[2]; _q4 = val[3]; }
		Quaternion(std::vector<float> val[4]) { _q1 = val->at(0); _q2 = val->at(1); _q3 = val->at(2); _q4 = val->at(3); }

		float q1() { return _q1; }
		float q2() { return _q2; }
		float q3() { return _q3; }
		float q4() { return _q4; }

		void q1(float q1) { _q1 = q1; }
		void q2(float q2) { _q2 = q2; }
		void q3(float q3) { _q3 = q3; }
		void q4(float q4) { _q4 = q4; }

		float& operator[] (size_t i)
		{
			switch (i) {
			case 0: return _q1;
			case 1: return _q2;
			case 2: return _q3;
			case 3: return _q4;
			default: throw "Valid indices: 0, 1, 2";
			}
		}

	private:
		float _q1;
		float _q2;
		float _q3;
		float _q4;

	};

}