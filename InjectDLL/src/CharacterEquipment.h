#pragma once

#include <cstddef> // For std::byte (C++17)

namespace DataTypes
{
    class CharacterEquipment
    {
    public:
        short WType;
        short Sword;
        short Shield;
        short Bow;
        short Head;
        short Upper;
        short Lower;
    };
}