#pragma once

#include <cstddef> // For std::byte (C++17)

namespace DataTypes
{
    class CharacterEquipment
    {
    public:
        std::byte WType; // Use std::byte for better type safety (C++17)
        short Sword;
        short Shield;
        short Bow;
        short Head;
        short Upper;
        short Lower;
    };
}