#include "Memory.h"

using namespace Memory;

void World_class::UpdateTime()
{

    WorldTime = Memory::read_bigEndianFloat(timeAddr, __FUNCTION__);
    WorldDay = Memory::read_bigEndian4Bytes(dayAddr, __FUNCTION__);

}

void World_class::SetWorldTime(float serverTime, int serverDay)
{

    if (serverDay > WorldDay || WorldDay - serverDay > 1 || (WorldDay == serverDay && serverTime - WorldTime > 2))
    {

        WorldDay = serverDay;
        WorldTime = serverTime;

        Memory::write_bigEndian4Bytes(dayAddr, WorldDay, __FUNCTION__);
        Memory::write_bigEndianFloat(timeAddr, WorldTime, __FUNCTION__);

    }

}

std::string World_class::GetTime()
{

    return to_string_precision(WorldTime, 4);

}

std::string World_class::GetDay()
{

    return std::to_string(WorldDay);

}

std::string World_class::to_string_precision(float number, int precision)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precision) << number;
    return stream.str();
}