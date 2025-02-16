#include "Vec3fBE.h"

using namespace DataTypes;

Vec3fBE::Vec3fBE()
{
	_x = new BigEndian<float>();
	_y = new BigEndian<float>();
	_z = new BigEndian<float>();
}

Vec3fBE::Vec3fBE(uint64_t address, const char* caller)
{
	if (address == 0)
	{
		_x = new BigEndian<float>(0, caller);
		_y = new BigEndian<float>(0, caller);
		_z = new BigEndian<float>(0, caller);
		return;
	}

	_x = new BigEndian<float>(address, caller);
	_y = new BigEndian<float>(address + 0x4, caller);
	_z = new BigEndian<float>(address + 0x8, caller);
}

void Vec3fBE::setAddress(uint64_t address, const char* caller, bool Validate)
{
	if (address == 0)
	{
		_x->setAddress(0, caller);
		_y->setAddress(0, caller);
		_z->setAddress(0, caller);
		return;
	}

	_x->setAddress(address, caller, Validate);
	_y->setAddress(address + 0x4, caller, false);
	_z->setAddress(address + 0x8, caller, false);
}

void Vec3fBE::set(Vec3f input, const char* caller)
{
	_x->set(input.x(), caller);
	_y->set(input.y(), caller);
	_z->set(input.z(), caller);

	LastKnown = input;
}

void Vec3fBE::set_x(float value, const char* caller)
{
	_x->set(value, caller);

	LastKnown.x(value);
}

void Vec3fBE::set_y(float value, const char* caller)
{
	_y->set(value, caller);

	LastKnown.y(value);
}

void Vec3fBE::set_z(float value, const char* caller)
{
	_z->set(value, caller);

	LastKnown.z(value);
}

Vec3f Vec3fBE::get(const char* caller)
{
	LastKnown = Vec3f(_x->get(caller), _y->get(caller), _z->get(caller));
	return LastKnown;
}

float Vec3fBE::get_x(const char* caller)
{
	LastKnown.x(_x->get(caller));
	return LastKnown.x();
}

float Vec3fBE::get_y(const char* caller)
{
	LastKnown.y(_y->get(caller));
	return LastKnown.y();
}

float Vec3fBE::get_z(const char* caller)
{
	LastKnown.z(_z->get(caller));
	return LastKnown.z();
}