#pragma once
#include "BigEndian.h"
#include "Vec3f.h"

namespace DataTypes
{
	class Vec3fBE
	{
	public:
		Vec3f LastKnown = Vec3f();

		Vec3fBE();
		Vec3fBE(uint64_t address, const char* caller);

		void setAddress(uint64_t address, const char* caller, bool Validate = true);

		void set(Vec3f input, const char* caller);
		void set_x(float value, const char* caller);
		void set_y(float value, const char* caller);
		void set_z(float value, const char* caller);

		Vec3f get(const char* caller);
		float get_x(const char* caller);
		float get_y(const char* caller);
		float get_z(const char* caller);

	private:
		BigEndian<float>* _x;
		BigEndian<float>* _y;
		BigEndian<float>* _z;
	};
}