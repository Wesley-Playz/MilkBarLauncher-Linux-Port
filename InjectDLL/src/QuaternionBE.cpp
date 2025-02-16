#include "QuaternionBE.h"

using namespace DataTypes;

QuaternionBE::QuaternionBE(uint64_t address, const char* caller)
{
	if (address == 0)
	{
		_q1 = new BigEndian<float>(0, caller);
		_q2 = new BigEndian<float>(0, caller);
		_q3 = new BigEndian<float>(0, caller);
		_q4 = new BigEndian<float>(0, caller);
		return;
	}

	_q1 = new BigEndian<float>(address, caller);
	_q2 = new BigEndian<float>(address + 0xC, caller);
	_q3 = new BigEndian<float>(address + 0x20, caller);
	_q4 = new BigEndian<float>(address + 0x2B, caller);
}

void QuaternionBE::setAddress(uint64_t address, const char* caller, bool Validate)
{
	if (address == 0)
	{
		_q1->setAddress(0, caller);
		_q2->setAddress(0, caller);
		_q3->setAddress(0, caller);
		_q4->setAddress(0, caller);
		return;
	}

	_q1->setAddress(address, caller, Validate);
	_q2->setAddress(address + 0x4, caller, false);
	_q3->setAddress(address + 0x8, caller, false);
	_q4->setAddress(address + 0xC, caller, false);
}

void QuaternionBE::set(Quaternion input, const char* caller)
{
	_q1->set(input.q1(), caller);
	_q2->set(input.q2(), caller);
	_q3->set(input.q3(), caller);
	_q4->set(input.q4(), caller);

	LastKnown = input;
}

void QuaternionBE::set_q1(float value, const char* caller)
{
	_q1->set(value, caller);
	LastKnown.q1(value);
}

void QuaternionBE::set_q2(float value, const char* caller)
{
	_q2->set(value, caller);
	LastKnown.q2(value);
}

void QuaternionBE::set_q3(float value, const char* caller)
{
	_q3->set(value, caller);
	LastKnown.q3(value);
}

void QuaternionBE::set_q4(float value, const char* caller)
{
	_q4->set(value, caller);
	LastKnown.q4(value);
}

Quaternion QuaternionBE::get(const char* caller)
{
	LastKnown = Quaternion(_q1->get(caller), _q2->get(caller), _q3->get(caller), _q4->get(caller));
	return LastKnown;
}

float QuaternionBE::get_q1(const char* caller)
{
	LastKnown.q1(_q1->get(caller));
	return LastKnown.q1();
}

float QuaternionBE::get_q2(const char* caller)
{
	LastKnown.q2(_q2->get(caller));
	return LastKnown.q2();
}

float QuaternionBE::get_q3(const char* caller)
{
	LastKnown.q3(_q3->get(caller));
	return LastKnown.q3();
}

float QuaternionBE::get_q4(const char* caller)
{
	LastKnown.q4(_q4->get(caller));
	return LastKnown.q4();
}