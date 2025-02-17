#pragma once

#include <cstdint>
#include <array>
#include <algorithm>
#include <string>
#include <string_view>
#include <vector>
#include <sys/mman.h> // For memory management on Linux
#include <unistd.h>   // For usleep
#include <fcntl.h>    // For file operations
#include <iostream>   // For logging
#include <stdexcept>  // For exceptions

class MemoryInstance {
public:
    MemoryInstance(void* cemuModuleHandle)
    {
        // Constructor implementation (if needed)
    }

    template <typename T>
    static void memory_swapEndianness(T& val) {
        union U {
            T val;
            std::array<std::uint8_t, sizeof(T)> raw;
        } src, dst;

        memcpy(&src.val, &val, sizeof(val));
        std::reverse_copy(src.raw.begin(), src.raw.end(), dst.raw.begin());
        memcpy(&val, &dst.val, sizeof(val));
    }

    template <typename T>
    void memory_readMemory(uint64_t offset, T* outPtr, uint64_t baseAddress) {
        memcpy(outPtr, reinterpret_cast<void*>(offset + baseAddress), sizeof(T));
    }

    template <typename T>
    void memory_readMemoryBE(uint64_t offset, T* outPtr, uint64_t baseAddress) {
        memcpy(outPtr, reinterpret_cast<void*>(offset + baseAddress), sizeof(T));
        memory_swapEndianness(*outPtr);
    }

    template <typename T>
    void memory_writeMemory(uint64_t offset, T value, uint64_t baseAddress) {
        memcpy(reinterpret_cast<void*>(offset + baseAddress), &value, sizeof(T));
    }

    template <typename T>
    void memory_writeMemoryBE(uint64_t offset, T value, uint64_t baseAddress) {
        memory_swapEndianness(value);
        memcpy(reinterpret_cast<void*>(offset + baseAddress), &value, sizeof(T));
    }

    template <size_t L>
    size_t memory_aobScan(uint8_t(&scanData)[L], size_t startOffset, size_t endOffset) {
        size_t endAddr = baseAddr + endOffset;
        int thing = sizeof(scanData);
        for (int addr = startOffset; addr < endOffset; addr = addr + 0x1) {
            uint8_t memorySegment[sizeof(scanData)];
            memory_readMemory(addr, &memorySegment);
            bool matches = true;
            for (int i = 0; i < sizeof(scanData); i++) {
                if (memorySegment[i] != scanData[i]) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                return addr;
            }
        }
        return 0;
    }

    template <typename T>
    uint64_t GetAddressFromSignature(std::vector<T> signature, uint64_t startaddress = 0, uint64_t endaddress = 0) {
        if (startaddress == 0) {
            startaddress = baseAddr + 0x10000000; // Default start address
        }
        if (endaddress == 0) {
            endaddress = baseAddr + 0x40000000; // Default end address
        }

        std::ifstream mapsFile("/proc/self/maps");
        std::string line;
        while (std::getline(mapsFile, line)) {
            size_t dashPos = line.find('-');
            if (dashPos != std::string::npos) {
                uint64_t regionStart = std::stoull(line.substr(0, dashPos), nullptr, 16);
                uint64_t regionEnd = std::stoull(line.substr(dashPos + 1, line.find(' ') - dashPos - 1), nullptr, 16);

                if (regionStart >= startaddress && regionEnd <= endaddress) {
                    for (uint64_t addr = regionStart; addr < regionEnd - signature.size(); addr++) {
                        bool match = true;
                        for (size_t i = 0; i < signature.size(); i++) {
                            if (signature[i] != -1 && signature[i] != *reinterpret_cast<uint8_t*>(addr + i)) {
                                match = false;
                                break;
                            }
                        }
                        if (match) {
                            return addr - baseAddr;
                        }
                    }
                }
            }
        }
        return 0;
    }

    class floatBE {
    public:
        floatBE() {
            data = 0.f;
        }
        floatBE(float input) {
            float copy = input;
            memory_swapEndianness(copy);
            data = copy;
        }

        void operator=(const float val) {
            float copy = val;
            memory_swapEndianness(copy);
            data = copy;
        }
        void operator=(const floatBE val) {
            data = val.data;
        }
        floatBE operator*(float val) {
            float dataCopy = data;
            memory_swapEndianness(dataCopy);
            dataCopy = dataCopy * val;
            return floatBE(dataCopy);
        }
        floatBE operator*(double val) {
            return operator*(static_cast<float>(val));
        }
        operator float() const {
            float copy = data;
            memory_swapEndianness(copy);
            return copy;
        }
    private:
        float data; // This is the big endian data, we're just using the float type because it has the right amount of bytes.
    };

    class intBE {
    public:
        void operator=(const int val) {
            data = val;
            memory_swapEndianness(data);
        }
        void operator=(const intBE val) {
            data = val.data;
        }
        operator int() const {
            int copy = data;
            memory_swapEndianness(copy);
            return copy;
        }
    private:
        int data; // This is the big endian data, we're just using the int type because it has the right amount of bytes.
    };

    struct {
        floatBE* PosX;
        floatBE* PosY;
        floatBE* PosZ;
        floatBE* VelX;
        floatBE* VelY;
        floatBE* VelZ;
        uint8_t* Health;
        floatBE* Stamina;
        floatBE* StaminaMax;
        floatBE* ActionSpeed;
        floatBE* SoundSpeed;
        floatBE* RunSpeed;
        struct {
            const bool* B_Held;
            const intBE* KeyComboCode;
        } controlData;
    } linkData;

    struct {
        floatBE* WorldSpeed;
        floatBE* WorldSoundSpeed;
        floatBE* TimeOfDay;
    } gameData;

    uint64_t baseAddr;
private:
    typedef void* (*memory_getBaseType)();
};