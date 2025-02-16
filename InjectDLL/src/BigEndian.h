#pragma once

#include "Memory.h"
#include <array>

namespace DataTypes
{
	template <typename T>
	class BigEndian
	{
	public:
		T LastKnown;

		BigEndian();
		BigEndian(uint64_t Address, const char* caller);
		BigEndian(uint64_t baseAddr, std::vector<int> offsets, int finalOffset, const char* caller);
		BigEndian(std::vector<int> signature, uint64_t finalOffset, const char* caller, uint64_t scanOffset = -1);

		void setAddress(uint64_t Address, const char* caller, bool Validate = true);

		void set(T val, const char* caller);
		T get(const char* caller);

	private:
		DWORD* Pointer;
		uint64_t address;
		bool AddressSet = false;
		const char* callerFunction = "";

		void SwapEndianness(T& val);

		bool ValidateAddress(uint64_t Address, const char* caller);
	};

	template <typename T>
	BigEndian<T>::BigEndian()
	{

	}

	template <typename T>
	BigEndian<T>::BigEndian(uint64_t Address, const char* caller)
	{
		setAddress(Address, caller);
	}

	template <typename T>
	BigEndian<T>::BigEndian(uint64_t baseAddr, std::vector<int> offsets, int finalOffset, const char* caller)
	{
		uint64_t addr = Memory::ReadPointers(baseAddr, offsets, true) + finalOffset;

		setAddress(addr, caller);
	}

	template <typename T>
	BigEndian<T>::BigEndian(std::vector<int> signature, uint64_t finalOffset, const char* caller, uint64_t scanOffset)
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
			Logging::LoggerService::LogDebug("Failed to find address", __FUNCTION__);
			Sleep(1000);

			if (retries == 15)
			{
				Logging::LoggerService::LogError("Could not find location address. Closing...");
				throw L"Failed to find mod address. Make sure that the mod is installed correctly in BCML and that BCML, Extended Memory and Multiplayer Utilities graphics packs are enabled.";
			}

			retries++;
			addr = Memory::PatternScan(signature, Memory::getBaseAddress(), 8, scanOffset) + finalOffset;
		}

		if (saveOffset)
			Memory::ScanOffset = addr - Memory::RegionStart;

		setAddress(addr, caller);
	}

	template <typename T>
	void BigEndian<T>::setAddress(uint64_t Address, const char* caller, bool Validate)
	{
		if (Validate)
			AddressSet = ValidateAddress(Address, caller);
		else
			AddressSet = true;

		if (AddressSet)
		{
			this->Pointer = (DWORD*)(Address);
			this->address = Address;
		}
	}

	template <typename T>
	void BigEndian<T>::set(T val, const char* caller)
	{
		LastKnown = val;

		if (!AddressSet)
		{
			return;
		}

		SwapEndianness(val);
		memcpy(Pointer, &val, sizeof(T));
	}

	template <typename T>
	T BigEndian<T>::get(const char* caller)
	{
		if (!AddressSet)
		{
			return (T)0;
		}

		T val;

		memcpy(&val, Pointer, sizeof(T));
		SwapEndianness(val);

		LastKnown = val;

		return val;
	}

	template <typename T>
	void BigEndian<T>::SwapEndianness(T& val)
	{
		union U {
			T val;
			std::array<std::uint8_t, sizeof(T)> raw;
		} src, dst;

		memcpy(&src.val, &val, sizeof(val));
		std::reverse_copy(src.raw.begin(), src.raw.end(), dst.raw.begin());
		memcpy(&val, &dst.val, sizeof(val));
	}

	template <typename T>
	bool BigEndian<T>::ValidateAddress(uint64_t Address, const char* caller)
	{
		if (Address == 0)
		{
			return false;
		}

		MEMORY_BASIC_INFORMATION mbi{ 0 };
		DWORD protectflags = (PAGE_GUARD | PAGE_NOCACHE | PAGE_NOACCESS);

		if (VirtualQuery((LPCVOID)Address, &mbi, sizeof(mbi)))
		{
			if (mbi.Protect & protectflags || !(mbi.State & MEM_COMMIT)) {
				Logging::LoggerService::LogError("Failed to validate address. Address: " + std::to_string(Address), caller);

				exit(1);
			}
		}

		return true;
	}

}