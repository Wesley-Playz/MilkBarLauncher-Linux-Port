#pragma once
#include "BigEndian.h"
#include "Quaternion.h"

namespace DataTypes
{
	class QuaternionBE
	{
	public:
		Quaternion LastKnown = Quaternion();

		QuaternionBE(uint64_t address, const char* caller);

		void setAddress(uint64_t address, const char* caller, bool Validate = true);

		void set(Quaternion input, const char* caller);
		void set_q1(float value, const char* caller);
		void set_q2(float value, const char* caller);
		void set_q3(float value, const char* caller);
		void set_q4(float value, const char* caller);

		Quaternion get(const char* caller);
		float get_q1(const char* caller);
		float get_q2(const char* caller);
		float get_q3(const char* caller);
		float get_q4(const char* caller);

	private:
		BigEndian<float>* _q1;
		BigEndian<float>* _q2;
		BigEndian<float>* _q3;
		BigEndian<float>* _q4;
	};
}