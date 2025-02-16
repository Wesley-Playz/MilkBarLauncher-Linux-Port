#pragma once
#include "Vec3fBE.h"

namespace DataTypes
{
	enum BombStatus : byte
	{
		Normal,
		Exploded,
		Cancelled,
		Deallocated,
		Processing
	};

	class BombAccess
	{
		Vec3fBE* Position = new Vec3fBE(0, "BombInstantiate");
		std::shared_mutex Mutex;

	public:
		Vec3f LastKnown = Vec3f();
		BombStatus Status = Deallocated;
		uint64_t BaseAddr = 0;

		Vec3f get(const char* caller)
		{
			Mutex.lock();
			if (Status == Exploded || Status == Deallocated)
				LastKnown = Vec3f(0, 0, 0);
			else if (Status == Cancelled)
				LastKnown = Vec3f(-1, -1, -1);
			else
			{
				// There's a weird issue where bombs will keep a value of 0.25, 0.2 or something like that when spawning, therefore
				Vec3f BombPos = Position->get(caller);

				if (BombPos.x() < 0.5 && BombPos.x() > -0.5 &&
					BombPos.y() < 0.5 && BombPos.y() > -0.5 &&
					BombPos.z() < 0.5 && BombPos.z() > -0.5)
					LastKnown = Vec3f(0, 0, 0);
				else
					LastKnown = Position->get(caller);
			}
			Mutex.unlock();

			return LastKnown;
		}

		void set(Vec3f newPosition, const char* caller)
		{
			Mutex.lock();
			LastKnown = newPosition;
			Position->set(newPosition, caller);
			Mutex.unlock();
		}
	
		void reset()
		{
			if (this->getStatus() != Normal)
				return;

			Mutex.lock();
			Position->set(LastKnown, __FUNCTION__);
			Mutex.unlock();
		}

		void setAddress(uint64_t addr, const char* caller)
		{
			Mutex.lock();
			if (addr != 0)
			{
				Position->setAddress(Memory::ReadPointers(addr, { 0x3A0, 0x50 - 0x4, 0x4 + 0x8, 0x80, 0x0, 0x5C, 0x18 }, true) + 0x50, caller);
				BaseAddr = addr;
			}
			else
			{
				Position->setAddress(0, caller);
				BaseAddr = 0;
			}
			Mutex.unlock();
		}

		void changeState(BombStatus newStatus)
		{
			Mutex.lock();
			this->Status = newStatus;
			Mutex.unlock();
		}

		BombStatus getStatus()
		{
			BombStatus response;
			Mutex.lock();
			response = this->Status;
			Mutex.unlock();
			return response;
		}
	};
}