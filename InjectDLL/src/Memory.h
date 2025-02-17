#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <any>
#include <map>
#include <regex>
#include <mutex>
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "LoggerService.h"
#include "ClientDTO.h"

#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <pthread.h>

namespace Memory
{
    static uint64_t base_addr = 0;
    static uint64_t ScanOffset = 0;
    static uint64_t RegionStart = 0;

    ////////////////// Scanner.cpp //////////////////
    uint64_t PatternScan(std::vector<int> signature, uint64_t baseAddr, int region = 0, uint64_t regionOffset = 0, bool Multiple = false, bool multipleRegions = false, uint64_t regionMaxOffset = 0);
    uint64_t TryPatternScan(std::vector<int> signature, uint64_t baseAddr, int region = 0, uint64_t regionOffset = 0, bool Multiple = false, bool multipleRegions = false, uint64_t regionMaxOffset = 0, int retries = 0, std::string flagName = "");
    std::vector<uint64_t> PatternScanMultiple(std::vector<int> signature, uint64_t baseAddr, int region = 0, uint64_t regionOffset = 0, bool multipleRegions = false, uint64_t regionMaxOffset = 0, int expectedValues = 0);
    uint64_t findRegionBaseAddress(uint64_t baseAddr, int region);

    ////////////////// ReadMemory.cpp //////////////////
    typedef void* (*memory_getBaseType)();
    uint64_t getBaseAddress();

    uint32_t read_memory(uint64_t Addr, const char* caller = "");
    int swap_Endian(int number);
    std::vector<uint8_t> read_bytes(uint64_t Addr, int bytes = 1, const char* caller = "");
    float read_bigEndianFloat(uint64_t Addr, const char* caller = "");
    int read_bigEndian4Bytes(uint64_t Addr, const char* caller = "");
    int read_bigEndian4BytesOffset(uint64_t Addr, const char* caller = "");
    std::string read_string(uint64_t Addr, int bytes = 50, const char* caller = "");
    void write_bigEndianFloat(uint64_t Addr, float value, const char* caller = "");
    void write_bigEndian4Bytes(uint64_t Addr, int value, const char* caller = "");
    void write_byte(uint64_t Addr, uint8_t byte, const char* caller = "");
    void write_bytes(uint64_t Addr, std::vector<uint8_t> bytes, const char* caller = "");
    void write_string(uint64_t Addr, std::string string, int bytes = 0, const char* caller = "");
    std::vector<uint8_t> getNop(int length);
    std::string hexStr(std::vector<uint8_t> data);
    std::string extractLocName(uint64_t Addr, int bytes = 50);
    bool CompareSignatures(std::vector<uint8_t> First, std::vector<uint8_t> Second, std::vector<int> WildCards = {  });
    uint64_t ReadPointers(uint64_t InitialAddress, std::vector<int> readingOffsets, bool IncludeBaseAddress = false);
    void ValidateAddress(uint64_t address);

    ////////////////// CharacterClasses.cpp //////////////////
    class World_class
    {
    public:
        float WorldTime = 0;
        int WorldDay = 0;
        uint64_t timeAddr;
        uint64_t dayAddr;

        void UpdateTime();
        void SetWorldTime(float serverTime, int serverDay);
        std::string GetTime();
        std::string GetDay();
        std::string to_string_precision(float number, int precision);
    };

    class Link_class
    {
    public:
        float Position[3];
        float Speed[3] = { 0, 0, 0 };
        float Rotation;
        int Animation;
        int Health;
        float atkup;
        int def;
        int IsEquipped;
        std::string Map;
        std::string Section;
        std::string Town;
        std::vector<std::string> Equipment;
        std::map<std::string, std::string> lastEquipment;
        rapidjson::Document WeaponDamages;
        uint64_t PosAddr;
        uint64_t RotAddr;
        uint64_t SpdAddr;
        uint64_t HealthAddr;
        uint64_t AnimAddr;
        uint64_t locationAddr;
        uint64_t townAddr;
        uint64_t porchAddr;
        uint64_t equippedAddr;
        uint64_t atkupAddr;
        uint64_t defAddr;
        uint64_t EqAddr;
        uint32_t lastHealthUpdate;
        uint32_t lastHealthReduce;
        
        void UpdatePosition();
        void UpdateRotation();
        void UpdateSpeed();
        void UpdateAnimation();
        void UpdateLocation();
        int UpdateHealth();
        void UpdateAtkDef();
    };
}
