#pragma once

#include <vector>
#include <string>
#include <map>
#include <tuple>
#include <any>
#include <regex>
#include <sstream>
#include <iomanip>
#include <mutex>
#include "Memory.h"

using namespace Memory;

class BombSyncer
{
private:
    std::mutex BombMutex;
    std::map<std::string, uint64_t> Bombs;

    std::string to_string_precision(float number, int precision);

public:
    void FindBombs();
    std::tuple<std::vector<std::string>, std::vector<std::any>, std::vector<std::string>> GetBombPositions();
    uint64_t FindBombPosAddr(uint64_t baseAddr);
};