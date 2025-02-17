#pragma once

#include <sys/mman.h> // For memory management on Linux
#include <unistd.h>   // For usleep
#include <fcntl.h>    // For file operations
#include <iostream>   // For logging
#include <vector>
#include <stdexcept>  // For exceptions
#include <fstream>    // For reading /proc/self/maps

namespace DataTypes
{
    class LocationAccess
    {
    public:
        CharacterLocation LastKnown;

        LocationAccess()
        {
            std::vector<int> signature = { 0x03, -1, 0xB5, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x10, 0x54, 0xF6, 0x44, 0x10, 0x2B, 0x7E, 0x78, 0x00, 0x00, 0x00 };

            Memory::ScanOffset = 0xe000000;

            uint64_t addr = Memory::PatternScan(signature, Memory::getBaseAddress(), 8, Memory::ScanOffset);

            int retries = 0;

            while (addr < 30000)
            {
                usleep(2000000); // Sleep for 2 seconds (Linux uses microseconds)

                if (retries == 15)
                {
                    std::cerr << "Could not find location address. Closing..." << std::endl;
                    throw std::runtime_error("Failed to find mod address. Make sure that the mod is installed correctly in BCML and that BCML, Extended Memory and Multiplayer Utilities graphics packs are enabled.");
                }

                retries++;

                std::cerr << "Failed to find address: " << __FUNCTION__ << std::endl;
                addr = Memory::PatternScan(signature, Memory::getBaseAddress(), 8, Memory::ScanOffset);
            }

            // Determine the memory region start
            Memory::RegionStart = Memory::getBaseAddress();
            std::ifstream mapsFile("/proc/self/maps");
            std::string line;
            while (std::getline(mapsFile, line))
            {
                size_t dashPos = line.find('-');
                if (dashPos != std::string::npos)
                {
                    uint64_t startAddr = std::stoull(line.substr(0, dashPos), nullptr, 16);
                    uint64_t endAddr = std::stoull(line.substr(dashPos + 1, line.find(' ') - dashPos - 1), nullptr, 16);
                    if (startAddr <= Memory::RegionStart && Memory::RegionStart < endAddr)
                    {
                        Memory::RegionStart = endAddr;
                        break;
                    }
                }
            }

            Memory::ScanOffset = addr - Memory::RegionStart;

            setAddress(addr, __FUNCTION__, true);
        }

        void setAddress(uint64_t Address, const char* caller, bool Validate)
        {
            if (Validate)
                AddressSet = ValidateAddress(Address, caller);
            else
                AddressSet = true;

            if (AddressSet)
            {
                this->Address = Address;
                LastKnown = CharacterLocation();
                LastKnown.Map = "Empty";
                LastKnown.Section = "Empty";
            }
        }

        CharacterLocation get(const char* caller)
        {
            CharacterLocation result;

            if (!AddressSet)
            {
                std::cerr << "Tried to get value of not set address. Returning default: " << caller << std::endl;
                return result;
            }

            result.Map = Memory::extractLocName(Address + 0x14);
            result.Section = Memory::extractLocName(Address + 0x40);

            LastKnown = result;

            return result;
        }

    private:
        uint64_t Address;
        bool AddressSet = false;

        bool ValidateAddress(uint64_t Address, const char* caller)
        {
            if (Address == 0)
            {
                std::cerr << "Address set to 0: " << caller << std::endl;
                return false;
            }

            // Use mincore to check if the memory is valid
            unsigned char vec;
            if (mincore(reinterpret_cast<void*>(Address), sizeof(uint64_t), &vec) == -1)
            {
                std::cerr << "Failed to validate address: " << caller << std::endl;
                return false;
            }

            return true;
        }
    };
}