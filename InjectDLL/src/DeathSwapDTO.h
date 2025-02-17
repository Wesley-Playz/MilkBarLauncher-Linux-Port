#pragma once

#include <cstdint> // For uint8_t and other fixed-width integer types
#include "Vec3f.h" // Ensure Vec3f is platform-agnostic

using namespace DataTypes;

namespace DTO
{
    class DeathSwapDTO
    {
    public:
        uint8_t Phase = 0; // Use uint8_t instead of byte for cross-platform compatibility
        Vec3f Position;    // Position vector (platform-agnostic)
    };
}