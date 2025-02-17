#pragma once

#include <sys/mman.h> // For memory management on Linux
#include <unistd.h>   // For usleep
#include <fcntl.h>    // For file operations
#include <iostream>   // For logging
#include <cstring>    // For memcpy
#include <vector>
#include <stdexcept>  // For exceptions

namespace DataTypes
{
    template <typename T>
    class LittleEndian
    {
    public:
        uint64_t address;
        T LastKnown;

        LittleEndian();
        LittleEndian(uint64_t Address, const char* caller);
        LittleEndian(uint64_t baseAddr, std::vector<int> offsets, int finalOffset, const char* caller);
        LittleEndian(std::vector<int> signature, uint64_t finalOffset, const char* caller, uint64_t scanOffset = -1);

        void setAddress(uint64_t Address, const char* caller, bool Validate = true);

        void set(T val, const char* caller);
        T get(const char* caller);

    private:
        T* Pointer;
        bool AddressSet = false;
        const char* callerFunction = "";

        bool ValidateAddress(uint64_t Address, const char* caller);
    };

    template <typename T>
    LittleEndian<T>::LittleEndian()
    {
        // Default constructor
    }

    template <typename T>
    LittleEndian<T>::LittleEndian(uint64_t Address, const char* caller)
    {
        setAddress(Address, caller);
    }

    template <typename T>
    LittleEndian<T>::LittleEndian(uint64_t baseAddr, std::vector<int> offsets, int finalOffset, const char* caller)
    {
        uint64_t addr = Memory::ReadPointers(baseAddr, offsets, true) + finalOffset;
        setAddress(addr, caller);
    }

    template <typename T>
    LittleEndian<T>::LittleEndian(std::vector<int> signature, uint64_t finalOffset, const char* caller, uint64_t scanOffset)
    {
        bool saveOffset = false;
        if (scanOffset == (uint64_t)(-1))
        {
            scanOffset = Memory::ScanOffset;
            saveOffset = true;
        }

        uint64_t addr = Memory::PatternScan(signature, Memory::getBaseAddress(), 8, scanOffset) + finalOffset;

        int retries = 0;
        while (addr - finalOffset < 30000)
        {
            std::cerr << "Failed to find address: " << caller << std::endl;
            usleep(1000000); // Sleep for 1 second (Linux uses microseconds)

            if (retries == 15)
            {
                std::cerr << "Could not find location address. Closing..." << std::endl;
                throw std::runtime_error("Failed to find mod address. Make sure that the mod is installed correctly in BCML and that BCML, Extended Memory and Multiplayer Utilities graphics packs are enabled.");
            }

            retries++;
            addr = Memory::PatternScan(signature, Memory::getBaseAddress(), 8, scanOffset) + finalOffset;
        }

        if (saveOffset)
            Memory::ScanOffset = addr - Memory::RegionStart - 0x50;

        setAddress(addr, caller);
    }

    template <typename T>
    void LittleEndian<T>::setAddress(uint64_t Address, const char* caller, bool Validate)
    {
        if (Validate)
            AddressSet = ValidateAddress(Address, caller);
        else
            AddressSet = true;

        if (AddressSet)
        {
            this->Pointer = reinterpret_cast<T*>(Address);
            this->address = Address;
        }
    }

    template <typename T>
    void LittleEndian<T>::set(T val, const char* caller)
    {
        LastKnown = val;

        if (!AddressSet)
            return;

        memcpy(Pointer, &val, sizeof(T));
    }

    template <typename T>
    T LittleEndian<T>::get(const char* caller)
    {
        if (!AddressSet)
            return static_cast<T>(0);

        memcpy(&LastKnown, Pointer, sizeof(T));
        return LastKnown;
    }

    template <typename T>
    bool LittleEndian<T>::ValidateAddress(uint64_t Address, const char* caller)
    {
        if (Address == 0)
            return false;

        // Use mincore to check if the memory is valid
        unsigned char vec;
        if (mincore(reinterpret_cast<void*>(Address), sizeof(T), &vec) == -1)
        {
            std::cerr << "Failed to validate address: " << caller << std::endl;
            return false;
        }

        return true;
    }
}