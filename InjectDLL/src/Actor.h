#pragma once

#include "Vec3fBE.h"
#include "QuaternionBE.h"
#include "LittleEndian.h"
#include "LocationAccess.h"
#include "EquippedItems.h"
#include "BombAccess.h"
#include "EnemyAccess.h"
#include "QuestAccess.h"

#include "ClientDTO.h"
#include "ServerDTO.h"

using namespace DataTypes;

namespace MemoryAccess
{
    class Actor 
    {
    public:
        uint64_t baseAddr = 0;
        Vec3fBE* Position = new Vec3fBE(0, "Instantiate");
        QuaternionBE* Rotation1 = new QuaternionBE(0, "Instantiate");
        QuaternionBE* Rotation2 = new QuaternionBE(0, "Instantiate");
        QuaternionBE* Rotation3 = new QuaternionBE(0, "Instantiate");
        QuaternionBE* Rotation4 = new QuaternionBE(0, "Instantiate");
        QuaternionBE* Rotation5 = new QuaternionBE(0, "Instantiate");

        void setAddress(uint64_t addr)
        {
            baseAddr = addr;

            if (addr != 0)
            {
                uint64_t posAddr = Memory::ReadPointers(baseAddr, { 0x3A0, 0x50, 0x4, 0x80, 0x0, 0x5C, 0x18 }, true) + 0x50;

                if (posAddr < 30000)
                {
                    baseAddr = 0;
                    return;
                }

                Position->setAddress(posAddr, "Position", true);
                Position2->setAddress(posAddr - 0x10, "Position2", false);
                Position3->setAddress(posAddr - 0x20, "Position3", false);
                Rotation1->setAddress(posAddr - 0x30, "Rotation1", false);
                Rotation2->setAddress(posAddr - 0x40, "Rotation2", false);
                Rotation3->setAddress(posAddr - 0x50, "Rotation3", false);
                Rotation4->setAddress(posAddr + 0x10, "Rotation4", false);
                Rotation5->setAddress(posAddr + 0x20, "Rotation5", false);
            }
            else
            {
                Position->setAddress(0, "Position");
                Position2->setAddress(0, "Position2");
                Position3->setAddress(0, "Position3");
                Rotation1->setAddress(0, "Rotation1");
                Rotation2->setAddress(0, "Rotation2");
                Rotation3->setAddress(0, "Rotation3");
                Rotation4->setAddress(0, "Rotation4");
                Rotation5->setAddress(0, "Rotation5");
            }
        }

        void Teleport(Vec3f newPosition)
        {
            Position->set(newPosition, __FUNCTION__);
            Position2->set(newPosition, __FUNCTION__);
            Position3->set(newPosition, __FUNCTION__);
        }

    private:
        Vec3fBE* Position2 = new Vec3fBE(0, "Instantiate");
        Vec3fBE* Position3 = new Vec3fBE(0, "Instantiate");
    };
}